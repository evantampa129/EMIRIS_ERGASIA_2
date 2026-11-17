#pragma once

#include "index.hpp"
#include <cstdint>

// inverted file with product quantization
// coarse centroids as in ivfflat (k-means on a sample), then every vector is stored as the code of its residual r = x - c(x)
// the residual is split into M subvectors of K/M coordinates, each one is replaced by the index of its
// nearest of the 2^nbits centroids of that subspace (codebook)
// a query computes, for every probed list, a lookup table of the distances of its own residual
// from all codewords and the distance of a point is the sum of M table values (asymmetric distance computation)
// the returned distances are these approximations, there is no exact re-ranking
class IVFPQ : public SearchIndex {
public:
    // kclusters <= 0 means defaultClusters(n)
    // throws std::invalid_argument if nprobe <= 0, M <= 0 or nbits not in [1, 8]
    IVFPQ(int kclusters, int nprobe, int M, int nbits, unsigned seed);

    // throws std::invalid_argument if the dimension K is not divisible by M
    void build(const Matrix& database) override;
    std::vector<Neighbor> search(const std::vector<float>& query, int k) const override;
    std::string name() const override { return "IVFPQ"; }

    // number of coarse clusters actually used after build
    int clusters() const { return static_cast<int>(centroids.size()); }

    // vector rebuilt from the coarse centroid and the codewords of point i, c(x) + q(r)
    std::vector<float> reconstruct(int i) const;

private:
    int kclusters;                       // requested number of coarse clusters, <= 0 for the default
    int nprobe;                          // number of nearest lists checked
    int M;                               // number of subvectors
    int nbits;                           // every subspace has 2^nbits codewords
    unsigned seed;                       // fixed seed of all k-means runs
    int sub_dim = 0;                     // K / M coordinates per subvector
    Matrix centroids;                    // coarse centroids
    std::vector<std::vector<int>> lists; // lists[c] has the indices of the vectors of cluster c
    std::vector<int> cluster_of;         // coarse cluster of every point
    std::vector<Matrix> codebooks;       // codebooks[m][j] is codeword j of subspace m
    std::vector<uint8_t> codes;          // codes[i * M + m] is the codeword of subvector m of point i
};
