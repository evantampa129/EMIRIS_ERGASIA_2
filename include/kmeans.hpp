#pragma once

#include "distance.hpp"
#include <vector>

// result of k-means clustering
struct KMeansResult {
    Matrix centroids;              // k centroids of the same dimension as the data
    std::vector<int> assignments;  // assignments[i] is the cluster of data[i]
};

// k-means with k-means++ initialization and lloyd iterations, generic for any dimension
// stops when no assignment changes or after max_iters iterations (25 is also the default of faiss)
// throws std::invalid_argument if data is empty or k is not in [1, n]
KMeansResult kmeans(const Matrix& data, int k, unsigned seed, int max_iters = 25);

// index of the centroid closest to x
int nearestCentroid(const Matrix& centroids, const std::vector<float>& x);
