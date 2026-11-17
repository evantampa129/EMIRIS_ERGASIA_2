#pragma once

#include <random>
#include <vector>

// one random projection h(x) = floor((v·x + t) / w) used by lsh and hypercube
// v has N(0,1) coordinates and t is uniform in [0, w)
struct RandomProjection {
    std::vector<float> v;  // random line on which x is projected
    float t = 0.0f;        // random shift of the cells on the line
};

// creates count projections for vectors of dimension dim, the rng is given so the seed is fixed by the caller
std::vector<RandomProjection> makeProjections(int count, int dim, double w, std::mt19937& rng);

// h(x) = floor((v·x + t) / w), the cell of the projected point on the line
long long projectionHash(const RandomProjection& p, const std::vector<float>& x, double w);
