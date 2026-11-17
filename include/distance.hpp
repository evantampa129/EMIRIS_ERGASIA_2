#pragma once

#include <vector>

// a set of vectors, each row is one image vector x_I in R^K
using Matrix = std::vector<std::vector<float>>;

// one search result, the position of the vector in the database and its distance from the query
struct Neighbor {
    int index;   // index of the vector in the database
    float dist;  // squared euclidean distance from the query (or its approximation)
};

// squared euclidean distance, keeps the same order as the real distance without the sqrt
// throws std::invalid_argument if the vectors have different dimensions
float squaredDistance(const std::vector<float>& a, const std::vector<float>& b);

// real euclidean distance d(x,y)
float euclideanDistance(const std::vector<float>& a, const std::vector<float>& b);

// true if neighbor a should be ranked before b, ties are broken by the smaller index
bool closerNeighbor(const Neighbor& a, const Neighbor& b);
