#include "ivfpq.hpp"
#include "exact.hpp"
#include "ivfflat.hpp"
#include "kmeans.hpp"
#include <algorithm>
#include <stdexcept>

// check and save the parameters, nbits <= 8 so a code fits in one byte
IVFPQ::IVFPQ(int kclusters, int nprobe, int M, int nbits, unsigned seed)
    : kclusters(kclusters), nprobe(nprobe), M(M), nbits(nbits), seed(seed) {
    if (nprobe <= 0 || M <= 0 || nbits < 1 || nbits > 8) {
        throw std::invalid_argument("Error: IVFPQ needs nprobe > 0, M > 0 and 1 <= nbits <= 8");
    }
}

// coarse clustering, residuals, one codebook per subspace and the code of every point
void IVFPQ::build(const Matrix& database) {
    centroids.clear();
    lists.clear();
    cluster_of.clear();
    codebooks.clear();
    codes.clear();
    if (database.empty()) {
        return;
    }

    int n = static_cast<int>(database.size());
    int dim = static_cast<int>(database[0].size());
    if (dim % M != 0) {
        throw std::invalid_argument("Error: the dimension K must be divisible by M");
    }
    sub_dim = dim / M;

    // coarse quantizer, same as ivfflat, every vector is assigned to its nearest centroid
    centroids = trainCoarseCentroids(database, kclusters, seed);
    cluster_of.resize(n);
    lists.assign(centroids.size(), std::vector<int>());
    for (int i = 0; i < n; ++i) {
        cluster_of[i] = nearestCentroid(centroids, database[i]);
        lists[cluster_of[i]].push_back(i);
    }

    // residuals r = x - c(x)
    Matrix residuals(n, std::vector<float>(dim));
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < dim; ++j) {
            residuals[i][j] = database[i][j] - centroids[cluster_of[i]][j];
        }
    }

    // one k-means per subspace, with less than 2^nbits points every point becomes a codeword
    int codewords = std::min(1 << nbits, n);
    codes.assign(static_cast<size_t>(n) * M, 0);
    for (int m = 0; m < M; ++m) {
        Matrix sub(n, std::vector<float>(sub_dim));
        for (int i = 0; i < n; ++i) {
            std::copy(residuals[i].begin() + m * sub_dim, residuals[i].begin() + (m + 1) * sub_dim, sub[i].begin());
        }

        KMeansResult pq = kmeans(sub, codewords, seed + 1 + m);
        codebooks.push_back(pq.centroids);
        for (int i = 0; i < n; ++i) {
            codes[static_cast<size_t>(i) * M + m] = static_cast<uint8_t>(pq.assignments[i]);
        }
    }
}

// c(x) plus the codeword of every subvector
std::vector<float> IVFPQ::reconstruct(int i) const {
    std::vector<float> x = centroids[cluster_of[i]];
    for (int m = 0; m < M; ++m) {
        const std::vector<float>& word = codebooks[m][codes[static_cast<size_t>(i) * M + m]];
        for (int j = 0; j < sub_dim; ++j) {
            x[m * sub_dim + j] += word[j];
        }
    }
    return x;
}

// asymmetric distance computation over the points of the nprobe nearest lists
std::vector<Neighbor> IVFPQ::search(const std::vector<float>& query, int k) const {
    std::vector<Neighbor> found;
    if (centroids.empty() || k <= 0) {
        return found;
    }
    if (query.size() != centroids[0].size()) {
        throw std::invalid_argument("Error: vectors have different dimensions");
    }

    std::vector<float> residual(query.size());
    std::vector<std::vector<float>> table(M);  // table[m][j] = ||r_m - codeword j of subspace m||^2

    for (const Neighbor& c : exactSearch(centroids, query, nprobe)) {
        // residual of the query depends on the centroid of the list, so the table is computed per list
        for (size_t j = 0; j < query.size(); ++j) {
            residual[j] = query[j] - centroids[c.index][j];
        }
        for (int m = 0; m < M; ++m) {
            std::vector<float> sub(residual.begin() + m * sub_dim, residual.begin() + (m + 1) * sub_dim);
            table[m].resize(codebooks[m].size());
            for (size_t j = 0; j < codebooks[m].size(); ++j) {
                table[m][j] = squaredDistance(sub, codebooks[m][j]);
            }
        }

        // distance of a point is the sum of M table lookups
        for (int p : lists[c.index]) {
            float dist = 0.0f;
            for (int m = 0; m < M; ++m) {
                dist += table[m][codes[static_cast<size_t>(p) * M + m]];
            }
            found.push_back({p, dist});
        }
    }

    // keep the k smallest approximate distances
    size_t keep = std::min(found.size(), static_cast<size_t>(k));
    std::partial_sort(found.begin(), found.begin() + keep, found.end(), closerNeighbor);
    found.resize(keep);
    return found;
}
