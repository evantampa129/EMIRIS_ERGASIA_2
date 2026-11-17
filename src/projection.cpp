#include "projection.hpp"
#include <cmath>
#include <stdexcept>

// draw v from N(0,1)^dim and t from U[0, w)
std::vector<RandomProjection> makeProjections(int count, int dim, double w, std::mt19937& rng) {
    std::normal_distribution<float> normal(0.0f, 1.0f);
    std::uniform_real_distribution<float> uniform(0.0f, static_cast<float>(w));

    std::vector<RandomProjection> projections(count);
    for (RandomProjection& p : projections) {
        p.v.resize(dim);
        for (float& coord : p.v) {
            coord = normal(rng);
        }
        p.t = uniform(rng);
    }
    return projections;
}

// dot product, shift and divide by the cell length
long long projectionHash(const RandomProjection& p, const std::vector<float>& x, double w) {
    if (p.v.size() != x.size()) {
        throw std::invalid_argument("Error: vectors have different dimensions");
    }

    double dot = 0.0;
    for (size_t i = 0; i < x.size(); ++i) {
        dot += static_cast<double>(p.v[i]) * x[i];
    }
    return static_cast<long long>(std::floor((dot + p.t) / w));
}
