#pragma once

#include <string>
#include <vector>

// this config struct stores user inputs
struct Config {

    // general parameters
    std::string hp_path;
    std::string mir_path;
    std::string split_file;  // name of  split file
    int vocab = 0;           // k-means center count (K)
    int S = 500;             // max descriptors to use for each image (S)
    int D = 0;               // number of MIRFlickr images to use (D)
    std::string query_set;   // which set is used for queries
    std::string output_file; // name of output file for results
    int seed = 1;            // randon seed
    std::string method;      // chosen search method

    // geometric verification parameters, distances in pixels
    int candidates = 50;
    double rho = 0.8;
    double tau = 3.0;
    int ransac_trials = 2000;
    double confidence = 0.99;
    bool adaptive = true;
    bool kclusters_given = false;

    // lsh parameters
    int lsh_k = 4;       // number of hi functions that make up each hash function g
    int lsh_L = 5;       // number of hash tables  used
    double lsh_w = 0.05; // length of the cell on the projection line

    // hypercube parameters
    int hc_kproj = 10;  // dimension d' of the hypercube where points are projected
    double hc_w = 0.05; // length of the cell on the projection line
    int hc_M = 500;     // max number of points to be checked
    int hc_probes = 10; // max number of hypercube vertices to be checked

    // ivfflat parameters
    int ivfflat_kclusters = 0; // number of index clusters
    int ivfflat_nprobe = 5;    // number of nearest  clusters to be checked during search

    // ivfpq parameters
    int ivfpq_kclusters = 0; // number of index clusters
    int ivfpq_nprobe = 5;    // number of clusters to be checked during IVFPQ search
    int ivfpq_M = 16;        // number of subvectors for Product Quantization, also K must be divisible by M
    int ivfpq_nbits = 8;     // each subspace is clusterd into 2^nbits clusters
};

// holds the folder names for each category of the dataset, commented expected numbers
struct DatasetSplit {
    std::vector<std::string> train;      // 70 names
    std::vector<std::string> validation; // 23 names
    std::vector<std::string> test;       // 23 names
};

// fill config struct with user inputs
Config parseArguments(int argc, char** argv);

// reads split.txt and categorizes each sequence folder into train, validation, or test
DatasetSplit readSplitFile(const std::string& filename);

// strict validation for the verify executable, before preprocessing
void validateVerificationConfig(const Config& config);
