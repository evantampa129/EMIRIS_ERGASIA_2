#include "kmeans.hpp"
#include <limits>
#include <random>
#include <stdexcept>

// linear scan over the centroids
int nearestCentroid(const Matrix& centroids, const std::vector<float>& x) {
    int best = 0;
    float best_dist = std::numeric_limits<float>::max();
    for (size_t c = 0; c < centroids.size(); ++c) {
        float d = squaredDistance(centroids[c], x);
        if (d < best_dist) {
            best_dist = d;
            best = static_cast<int>(c);
        }
    }
    return best;
}

// k-means++, the first centroid is random and every next one is chosen
// with probability proportional to its squared distance from the closest centroid so far
static Matrix initCentroids(const Matrix& data, int k, std::mt19937& rng) {
    size_t n = data.size();
    Matrix centroids;
    centroids.push_back(data[std::uniform_int_distribution<size_t>(0, n - 1)(rng)]);

    std::vector<double> min_dist(n);
    for (size_t i = 0; i < n; ++i) {
        min_dist[i] = squaredDistance(data[i], centroids[0]);
    }

    while (static_cast<int>(centroids.size()) < k) {
        double total = 0.0;
        for (double d : min_dist) {
            total += d;
        }

        // all remaining points are on top of a centroid, pick any of them
        size_t chosen = 0;
        if (total <= 0.0) {
            chosen = std::uniform_int_distribution<size_t>(0, n - 1)(rng);
        }
        else {
            double target = std::uniform_real_distribution<double>(0.0, total)(rng);
            double running = 0.0;
            chosen = n - 1;
            for (size_t i = 0; i < n; ++i) {
                running += min_dist[i];
                if (running >= target) {
                    chosen = i;
                    break;
                }
            }
        }

        centroids.push_back(data[chosen]);
        for (size_t i = 0; i < n; ++i) {
            double d = squaredDistance(data[i], centroids.back());
            if (d < min_dist[i]) {
                min_dist[i] = d;
            }
        }
    }
    return centroids;
}

// lloyd iterations, assign every point to its nearest centroid and move every centroid to the mean of its points
KMeansResult kmeans(const Matrix& data, int k, unsigned seed, int max_iters) {
    if (data.empty() || k < 1 || k > static_cast<int>(data.size())) {
        throw std::invalid_argument("Error: k-means needs a non empty data set and 1 <= k <= n");
    }

    std::mt19937 rng(seed);
    size_t n = data.size();
    size_t dim = data[0].size();

    KMeansResult result;
    result.centroids = initCentroids(data, k, rng);
    result.assignments.assign(n, -1);

    for (int iter = 0; iter < max_iters; ++iter) {
        // assignment step
        bool changed = false;
        for (size_t i = 0; i < n; ++i) {
            int c = nearestCentroid(result.centroids, data[i]);
            if (c != result.assignments[i]) {
                result.assignments[i] = c;
                changed = true;
            }
        }
        if (!changed) {
            break;
        }

        // update step, sums in double to reduce rounding errors
        std::vector<std::vector<double>> sums(k, std::vector<double>(dim, 0.0));
        std::vector<int> counts(k, 0);
        for (size_t i = 0; i < n; ++i) {
            int c = result.assignments[i];
            ++counts[c];
            for (size_t j = 0; j < dim; ++j) {
                sums[c][j] += data[i][j];
            }
        }

        // an empty cluster keeps its previous centroid
        for (int c = 0; c < k; ++c) {
            if (counts[c] == 0) {
                continue;
            }
            for (size_t j = 0; j < dim; ++j) {
                result.centroids[c][j] = static_cast<float>(sums[c][j] / counts[c]);
            }
        }
    }
    return result;
}
