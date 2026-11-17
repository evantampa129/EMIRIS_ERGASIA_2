#pragma once

#include "index.hpp"

// keeps the k closest of the given candidate indices, sorted by increasing distance
// used by exact search and for the re-ranking step of the approximate methods
std::vector<Neighbor> rankCandidates(const Matrix& database, const std::vector<float>& query,
                                     const std::vector<int>& candidates, int k);

// exact search for the k nearest neighbors with a linear scan over the whole database
std::vector<Neighbor> exactSearch(const Matrix& database, const std::vector<float>& query, int k);

// exhaustive search, no index structures, every query is compared with all n vectors
class ExactSearch : public SearchIndex {
public:
    void build(const Matrix& database) override;
    std::vector<Neighbor> search(const std::vector<float>& query, int k) const override;
    std::string name() const override { return "Exact Search"; }

private:
    const Matrix* data = nullptr; // database given in build
};
