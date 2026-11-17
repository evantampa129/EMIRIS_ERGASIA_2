#include "hypercube.hpp"
#include "exact.hpp"
#include <algorithm>
#include <numeric>
#include <stdexcept>

// splitmix64 mixing function, small changes of the input change about half the output bits
static uint64_t mix64(uint64_t x) {
    x += 0x9e3779b97f4a7c15ULL;
    x = (x ^ (x >> 30)) * 0xbf58476d1ce4e5b9ULL;
    x = (x ^ (x >> 27)) * 0x94d049bb133111ebULL;
    return x ^ (x >> 31);
}

// check and save the parameters, kproj <= 30 so a vertex fits in 32 bits
Hypercube::Hypercube(int kproj, double w, int M, int probes, unsigned seed)
    : kproj(kproj), w(w), M(M), probes(probes), seed(seed) {
    if (kproj < 1 || kproj > 30 || w <= 0.0 || M <= 0 || probes <= 0) {
        throw std::invalid_argument("Error: Hypercube needs 1 <= kproj <= 30 and w, M, probes > 0");
    }
}

// the bit depends only on (salt, i, h value), so the same h value always gives the same bit
int Hypercube::bitOf(int i, long long h_value) const {
    uint64_t key = salt ^ mix64(static_cast<uint64_t>(i) * 0x100000001b3ULL ^ static_cast<uint64_t>(h_value));
    return static_cast<int>(mix64(key) & 1ULL);
}

// bit i of the vertex is f_i(h_i(x))
uint32_t Hypercube::vertex(const std::vector<float>& x) const {
    uint32_t v = 0;
    for (int i = 0; i < kproj; ++i) {
        if (bitOf(i, projectionHash(h[i], x, w))) {
            v |= (1u << i);
        }
    }
    return v;
}

// create the projections and put every point in the bucket of its vertex
void Hypercube::build(const Matrix& database) {
    data = &database;
    buckets.clear();
    h.clear();
    if (database.empty()) {
        return;
    }

    std::mt19937 rng(seed);
    h = makeProjections(kproj, static_cast<int>(database[0].size()), w, rng);
    salt = (static_cast<uint64_t>(rng()) << 32) | rng();

    // the points enter their vertex in random order, so when M stops the search inside a vertex
    // the checked points do not depend on their position in the database (the hpatches images are first)
    std::vector<int> order(database.size());
    std::iota(order.begin(), order.end(), 0);
    std::shuffle(order.begin(), order.end(), rng);
    for (int p : order) {
        buckets[vertex(database[p])].push_back(p);
    }
}

// for every distance d = 0, 1, ..., kproj take all masks with d bits set in increasing order
// next mask with the same number of bits is found with gosper's hack
std::vector<uint32_t> Hypercube::probeOrder(uint32_t start, int kproj, int count) {
    std::vector<uint32_t> order;
    uint64_t limit = 1ULL << kproj;

    for (int d = 0; d <= kproj && static_cast<int>(order.size()) < count; ++d) {
        uint64_t mask = (1ULL << d) - 1;  // smallest mask with d bits set
        while (mask < limit && static_cast<int>(order.size()) < count) {
            order.push_back(start ^ static_cast<uint32_t>(mask));
            if (mask == 0) {
                break;  // d = 0 has only one mask
            }
            uint64_t lowest = mask & (~mask + 1);
            uint64_t ripple = mask + lowest;
            mask = (((ripple ^ mask) >> 2) / lowest) | ripple;
        }
    }
    return order;
}

// visit the vertices in hamming order and collect points until one of the two limits is reached
std::vector<int> Hypercube::candidates(const std::vector<float>& query) const {
    if (data == nullptr) {
        throw std::logic_error("Error: search called before build");
    }

    std::vector<int> result;
    if (data->empty()) {
        return result;
    }

    for (uint32_t v : probeOrder(vertex(query), kproj, probes)) {
        auto bucket = buckets.find(v);
        if (bucket == buckets.end()) {
            continue;
        }
        for (int p : bucket->second) {
            if (static_cast<int>(result.size()) == M) {
                return result;
            }
            result.push_back(p);
        }
    }
    return result;
}

// exact distances only for the candidates
std::vector<Neighbor> Hypercube::search(const std::vector<float>& query, int k) const {
    return rankCandidates(*data, query, candidates(query), k);
}
