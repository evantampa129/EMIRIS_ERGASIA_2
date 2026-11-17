#pragma once // include header once

#include <string>
#include <vector>
#include <opencv2/opencv.hpp> // needed for openCV data types like matrix
#include "config.hpp"

// struct that holds image id and final bovw vector for that image
struct ImageVector {
    std::string id;
    std::vector<float> vec;
};

// randomly selects up to S descriptors from a single image's descriptor matrix
// if the image has less than S, it selects all of them
std::vector<std::vector<float>> sampleDescriptors(
    const std::vector<std::vector<float>>& descriptors,
    int S,
    int seed
);

// loops through all 70 training folders, extracts SIFT, and runs k-means to find K centers
std::vector<std::vector<float>> buildVocabulary(
    const std::string& hp_path, 
    const std::vector<std::string>& train_seqs, 
    int vocab_size, 
    int S, 
    int seed
);

// extracts all SIFT descriptors from a single image and returns a standard vector of vectorsm, instead of openCV matrix
std::vector<std::vector<float>> extractSIFT(
    const std::string& img_path, 
    cv::Ptr<cv::SIFT>& sift
);

// find k cluster centers and return them in a matrix k x dims
std::vector<std::vector<float>> kmeans(
    const std::vector<std::vector<float>>& data,
    int K,
    int max_iter,
    double epsilon,
    int seed
);

// returns the BoVW vector for a single image.
// it is a frequency histogram of descriptors assigned to each cluster center,
// normalized so that all values sum to 1.
std::vector<float> computeBoVWVector(
    const std::vector<std::vector<float>>& descriptors,
    const std::vector<std::vector<float>>& centers
);

// convert images to BoVW vectors and store them in queries and database vectors
void buildQueriesAndDatabase(
    const Config& config,
    const std::vector<std::string>& active_seqs,
    const std::vector<std::vector<float>>& centers,
    std::vector<ImageVector>& queries,
    std::vector<ImageVector>& database
);