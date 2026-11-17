#pragma once

#include "index.hpp"
#include "projection.hpp"
#include <unordered_map>

// hashes the key of a bucket, the k values of g(x) = (h_1(x), ..., h_k(x))
struct BucketKeyHash {
    size_t operator()(const std::vector<long long>& key) const;
};

// locality sensitive hashing for the euclidean distance
// L hash tables, each one uses g(x) = (h_1(x), ..., h_k(x)) as bucket key
// candidates are the points that share a bucket with the query in any table, they are re-ranked exactly
class LSH : public SearchIndex {
public:
    // throws std::invalid_argument if k <= 0, L <= 0 or w <= 0
    LSH(int k, int L, double w, unsigned seed);

    void build(const Matrix& database) override;
    std::vector<Neighbor> search(const std::vector<float>& query, int k) const override;
    std::string name() const override { return "LSH"; }

    // distinct candidates of the query from all tables, used by search and by the tests
    std::vector<int> candidates(const std::vector<float>& query) const;

private:
    // g(x) for one table
    std::vector<long long> bucketKey(int table, const std::vector<float>& x) const;

    using Table = std::unordered_map<std::vector<long long>, std::vector<int>, BucketKeyHash>;

    int k;                                         // number of h functions in every g
    int L;                                         // number of hash tables
    double w;                                      // cell length on the projection line
    unsigned seed;                                 // fixed seed of the random projections
    const Matrix* data = nullptr;                  // database given in build
    std::vector<std::vector<RandomProjection>> h;  // h[table][i] is h_i of that table
    std::vector<Table> tables;                     // bucket key -> indices of the points in the bucket
};
