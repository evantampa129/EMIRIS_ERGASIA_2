#pragma once

#include "index.hpp"
#include "projection.hpp"
#include <cstdint>
#include <unordered_map>

// random projection on the hypercube {0,1}^kproj
// every h_i(x) is mapped to one bit f_i(h_i(x)) and the kproj bits form the vertex of the point
// the query checks its own vertex and then its neighbors in increasing hamming distance,
// until probes vertices or M candidate points have been checked
class Hypercube : public SearchIndex {
public:
    // throws std::invalid_argument if kproj is not in [1, 30] or w, M, probes are not positive
    Hypercube(int kproj, double w, int M, int probes, unsigned seed);

    void build(const Matrix& database) override;
    std::vector<Neighbor> search(const std::vector<float>& query, int k) const override;
    std::string name() const override { return "Hypercube"; }

    // vertex of x, bit i is f_i(h_i(x))
    uint32_t vertex(const std::vector<float>& x) const;

    // candidates of the query, at most M points from at most probes vertices
    std::vector<int> candidates(const std::vector<float>& query) const;

    // the first count vertices visited from start, in increasing hamming distance
    static std::vector<uint32_t> probeOrder(uint32_t start, int kproj, int count);

private:
    // f_i, maps a value of h_i to a random but fixed bit
    int bitOf(int i, long long h_value) const;

    int kproj;                         // dimension d' of the hypercube
    double w;                          // cell length on the projection line
    int M;                             // max number of candidate points
    int probes;                        // max number of vertices checked
    unsigned seed;                     // fixed seed of the projections and of f_i
    uint64_t salt = 0;                 // random value drawn from the seed, used by f_i
    const Matrix* data = nullptr;      // database given in build
    std::vector<RandomProjection> h;   // h_1 .. h_kproj
    std::unordered_map<uint32_t, std::vector<int>> buckets; // vertex -> indices of its points
};
