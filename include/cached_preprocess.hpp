#pragma once

#include "config.hpp"
#include "evaluation.hpp"
#include <string>
#include <vector>

// the preprocessing (SIFT, vocabulary, bag of visual words) does not depend on the search method,
// so its results are saved in cache_dir and the next runs with the same parameters only read them
// the files are named after K, S, seed and the split, so a change of any of them computes everything again

// vocabulary of K centers from the train sequences, loaded from the cache if it exists
Matrix loadOrBuildVocabulary(const Config& config, const DatasetSplit& split, const std::string& cache_dir);

// queries and database of the chosen set (-set), loaded from the cache if it exists
// the mirflickr vectors are shared by all sets and all D, the first D valid images are the first D of a larger D
void loadOrBuildVectors(const Config& config, const DatasetSplit& split, const Matrix& centers,
                        const std::string& cache_dir, VectorSet& queries, VectorSet& database);
