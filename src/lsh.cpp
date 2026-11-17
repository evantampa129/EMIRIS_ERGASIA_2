#include "lsh.hpp"
#include "exact.hpp"
#include <stdexcept>

// combine the hashes of the k values, same mixing idea as boost::hash_combine
size_t BucketKeyHash::operator()(const std::vector<long long>& key) const {
    size_t result = 0;
    for (long long value : key) {
        result ^= std::hash<long long>()(value) + 0x9e3779b97f4a7c15ULL + (result << 6) + (result >> 2);
    }
    return result;
}

// check and save the parameters, the tables are created in build
LSH::LSH(int k, int L, double w, unsigned seed) : k(k), L(L), w(w), seed(seed) {
    if (k <= 0 || L <= 0 || w <= 0.0) {
        throw std::invalid_argument("Error: LSH needs k > 0, L > 0 and w > 0");
    }
}

// g(x) = (h_1(x), ..., h_k(x)) of the given table
std::vector<long long> LSH::bucketKey(int table, const std::vector<float>& x) const {
    std::vector<long long> key(k);
    for (int i = 0; i < k; ++i) {
        key[i] = projectionHash(h[table][i], x, w);
    }
    return key;
}

// create the random projections and put every point in one bucket of every table
void LSH::build(const Matrix& database) {
    data = &database;
    h.clear();
    tables.assign(L, Table());
    if (database.empty()) {
        return;
    }

    std::mt19937 rng(seed);
    int dim = static_cast<int>(database[0].size());
    for (int table = 0; table < L; ++table) {
        h.push_back(makeProjections(k, dim, w, rng));
    }

    for (int table = 0; table < L; ++table) {
        for (size_t p = 0; p < database.size(); ++p) {
            tables[table][bucketKey(table, database[p])].push_back(static_cast<int>(p));
        }
    }
}

// union of the query buckets of all tables, every point appears once
std::vector<int> LSH::candidates(const std::vector<float>& query) const {
    if (data == nullptr) {
        throw std::logic_error("Error: search called before build");
    }

    std::vector<int> result;
    if (data->empty()) {
        return result;
    }

    std::vector<char> seen(data->size(), 0); // marks points already added from another table
    for (int table = 0; table < L; ++table) {
        auto bucket = tables[table].find(bucketKey(table, query));
        if (bucket == tables[table].end()) {
            continue;
        }
        for (int p : bucket->second) {
            if (!seen[p]) {
                seen[p] = 1;
                result.push_back(p);
            }
        }
    }
    return result;
}

// exact distances only for the candidates
std::vector<Neighbor> LSH::search(const std::vector<float>& query, int k) const {
    return rankCandidates(*data, query, candidates(query), k);
}
