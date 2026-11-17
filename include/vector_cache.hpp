#pragma once

#include "evaluation.hpp"
#include <string>

// binary file with the ids and the vectors of a VectorSet
// SIFT and the vocabulary take minutes, so they are computed once and reused by the next runs
// file format: count, dimension, then for every entry the id length, the id and dimension floats

// creates the folder of the file if needed, throws std::runtime_error if the file cannot be written
void saveVectorSet(const std::string& filename, const VectorSet& set);

// returns false if the file does not exist, throws std::runtime_error if it is damaged
bool loadVectorSet(const std::string& filename, VectorSet& set);
