#pragma once

#include "index.hpp"

// size of the random sample the index k-means is trained on, ceil(sqrt(n))
int trainingSampleSize(int n);

// default number of index clusters, round(n^(1/4)) but at least 5, the final value is chosen on the validation set
int defaultClusters(int n);

// kclusters centroids (<= 0 for the default) from k-means on a random sample of ceil(sqrt(n)) database vectors
// the sample grows to kclusters vectors if kclusters is larger, shared by ivfflat and ivfpq
Matrix trainCoarseCentroids(const Matrix& database, int kclusters, unsigned seed);

// inverted file flat
// k-means on a sample of the database vectors gives kclusters coarse centroids, every vector of the database
// is stored in the inverted list of its nearest centroid, a query checks the lists of its nprobe nearest centroids exactly
class IVFFlat : public SearchIndex {
public:
    // kclusters <= 0 means defaultClusters(n), throws std::invalid_argument if nprobe <= 0
    IVFFlat(int kclusters, int nprobe, unsigned seed);

    void build(const Matrix& database) override;
    std::vector<Neighbor> search(const std::vector<float>& query, int k) const override;
    std::string name() const override { return "IVFFlat"; }

    // number of clusters actually used after build
    int clusters() const { return static_cast<int>(centroids.size()); }

    // all points of the nprobe lists closest to the query
    std::vector<int> candidates(const std::vector<float>& query) const;

private:
    int kclusters;                       // requested number of clusters, <= 0 for the default
    int nprobe;                          // number of nearest lists checked
    unsigned seed;                       // fixed seed of k-means
    const Matrix* data = nullptr;        // database given in build
    Matrix centroids;                    // coarse centroids
    std::vector<std::vector<int>> lists; // lists[c] has the indices of the vectors of cluster c
};
