#include "ivfflat.hpp"
#include "exact.hpp"
#include "kmeans.hpp"
#include <algorithm>
#include <cmath>
#include <numeric>
#include <random>
#include <stdexcept>

// ceil(sqrt(n)) with integers, so rounding of sqrt cannot give a wrong value for perfect squares
int trainingSampleSize(int n) {
    int s = static_cast<int>(std::sqrt(static_cast<double>(n)));
    while (s * s < n) {
        ++s;
    }
    while (s > 1 && (s - 1) * (s - 1) >= n) {
        --s;
    }
    return s;
}

// round(n^(1/4)), at least 5 and at most n
int defaultClusters(int n) {
    int c = static_cast<int>(std::lround(std::pow(static_cast<double>(n), 0.25)));
    return std::min(std::max(c, 5), n);
}

// k-means on a random sample of about sqrt(n) vectors, k cannot be larger than the sample
Matrix trainCoarseCentroids(const Matrix& database, int kclusters, unsigned seed) {
    int n = static_cast<int>(database.size());
    std::vector<int> order(n);
    std::iota(order.begin(), order.end(), 0);
    std::mt19937 rng(seed);
    std::shuffle(order.begin(), order.end(), rng);

    int size = trainingSampleSize(n);
    int k = kclusters > 0 ? kclusters : defaultClusters(n);
    if (k > size) {
        size = std::min(k, n); // a larger sample so that every centroid can have a point
    }

    Matrix sample;
    sample.reserve(size);
    for (int i = 0; i < size; ++i) {
        sample.push_back(database[order[i]]);
    }
    return kmeans(sample, std::min(k, size), seed).centroids;
}

// check and save the parameters
IVFFlat::IVFFlat(int kclusters, int nprobe, unsigned seed) : kclusters(kclusters), nprobe(nprobe), seed(seed) {
    if (nprobe <= 0) {
        throw std::invalid_argument("Error: IVFFlat needs nprobe > 0");
    }
}

// cluster the database and fill the inverted lists
void IVFFlat::build(const Matrix& database) {
    data = &database;
    centroids.clear();
    lists.clear();
    if (database.empty()) {
        return;
    }

    // every vector of the database is assigned, also the ones of the sample
    centroids = trainCoarseCentroids(database, kclusters, seed);
    lists.assign(centroids.size(), std::vector<int>());
    for (size_t i = 0; i < database.size(); ++i) {
        lists[nearestCentroid(centroids, database[i])].push_back(static_cast<int>(i));
    }
}

// the nearest centroids are found with exact search over the centroids
std::vector<int> IVFFlat::candidates(const std::vector<float>& query) const {
    if (data == nullptr) {
        throw std::logic_error("Error: search called before build");
    }

    std::vector<int> result;
    for (const Neighbor& c : exactSearch(centroids, query, nprobe)) {
        result.insert(result.end(), lists[c.index].begin(), lists[c.index].end());
    }
    return result;
}

// exact distances only for the points of the probed lists
std::vector<Neighbor> IVFFlat::search(const std::vector<float>& query, int k) const {
    return rankCandidates(*data, query, candidates(query), k);
}
