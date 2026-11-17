#pragma once

#include "config.hpp"
#include "distance.hpp"
#include <memory>
#include <string>
#include <vector>

// common interface of all search methods (exact, lsh, hypercube, ivfflat, ivfpq)
// build() is called once with the database, search() once for every query
class SearchIndex {
public:
    virtual ~SearchIndex() = default;

    // builds the index structures, the database must stay alive while the index is used
    virtual void build(const Matrix& database) = 0;

    // returns up to k neighbors sorted by increasing distance, may return less than k
    virtual std::vector<Neighbor> search(const std::vector<float>& query, int k) const = 0;

    // method name printed in the output file
    virtual std::string name() const = 0;
};

// creates the search method chosen with -exact, -lsh, -hypercube, -ivfflat or -ivfpq and its parameters
// throws std::invalid_argument if no known method was given
std::unique_ptr<SearchIndex> createIndex(const Config& config);
