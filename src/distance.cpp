#include "distance.hpp"
#include <cmath>
#include <stdexcept>

// squared euclidean distance, sum of (a_i - b_i)^2
float squaredDistance(const std::vector<float>& a, const std::vector<float>& b) {
    if (a.size() != b.size()) {
        throw std::invalid_argument("Error: vectors have different dimensions");
    }

    float sum = 0.0f;
    for (size_t i = 0; i < a.size(); ++i) {
        float diff = a[i] - b[i];
        sum += diff * diff;
    }
    return sum;
}

// real euclidean distance d(x,y)
float euclideanDistance(const std::vector<float>& a, const std::vector<float>& b) {
    return std::sqrt(squaredDistance(a, b));
}

// smaller distance first, equal distances are ordered by index so results are deterministic
bool closerNeighbor(const Neighbor& a, const Neighbor& b) {
    if (a.dist != b.dist) {
        return a.dist < b.dist;
    }
    return a.index < b.index;
}
