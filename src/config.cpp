#include "config.hpp"
#include <stdexcept>
#include <fstream>

// fill config struct with user inputs
Config parseArguments(int argc, char** argv) {
    Config config; // empty to be filled config object
    
    // parse command line args
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i]; // convert arg to cpp string for comparison
        
        // if arg is path name or file name,  save it statically
        // if arg is a number, convert to int using stoi and save it  
        if (arg == "-hp" && i + 1 < argc) {
            config.hp_path = argv[++i];
        }
        else if (arg == "-mir" && i + 1 < argc) {
            config.mir_path = argv[++i];
        }
        else if (arg == "-split" && i + 1 < argc) {
            config.split_file = argv[++i];
        }
        else if (arg == "-vocab" && i + 1 < argc) {
            config.vocab = std::stoi(argv[++i]); 
        }
        else if (arg == "-S" && i + 1 < argc) {
            config.S = std::stoi(argv[++i]);
        }
        else if (arg == "-D" && i + 1 < argc) {
            config.D = std::stoi(argv[++i]);
        }
        else if (arg == "-set" && i + 1 < argc) {
            config.query_set = argv[++i];
        }
        else if (arg == "-o" && i + 1 < argc) {
            config.output_file = argv[++i];
        }
        else if (arg == "-seed" && i + 1 < argc) {
            config.seed = std::stoi(argv[++i]);
        }

        // save search method
        else if (arg == "-exact" || arg == "-lsh" || arg == "-hypercube" || arg == "-ivfflat" || arg == "-ivfpq") {
            config.method = arg;
        }

        // search method parameters
        else if (arg == "-k" && i + 1 < argc) {
            config.lsh_k = std::stoi(argv[++i]);
        }
        else if (arg == "-L" && i + 1 < argc) {
            config.lsh_L = std::stoi(argv[++i]);
        }
        else if (arg == "-kproj" && i + 1 < argc) {
            config.hc_kproj = std::stoi(argv[++i]);
        }
        else if (arg == "-probes" && i + 1 < argc) {
            config.hc_probes = std::stoi(argv[++i]);
        }
        else if (arg == "-nbits" && i + 1 < argc) {
            config.ivfpq_nbits = std::stoi(argv[++i]);
        }
        // because some parameters are shared, we set both of them to the same value for simplicitys sake
        // even though only one of them is used depending on the search method
        else if (arg == "-w" && i + 1 < argc) {
            double val = std::stod(argv[++i]); // stod converts string to double
            config.lsh_w = val;
            config.hc_w = val;
        }
        else if (arg == "-M" && i + 1 < argc) {
            int val = std::stoi(argv[++i]);
            config.hc_M = val;
            config.ivfpq_M = val;
        }
        else if (arg == "-kclusters" && i + 1 < argc) {
            int val = std::stoi(argv[++i]);
            config.ivfflat_kclusters = val;
            config.ivfpq_kclusters = val;
        }
        else if (arg == "-nprobe" && i + 1 < argc) {
            int val = std::stoi(argv[++i]);
            config.ivfflat_nprobe = val;
            config.ivfpq_nprobe = val;
        }
    }
    
    return config;
}

// reads split.txt and categorizes each sequence folder into train, validation, or test
DatasetSplit readSplitFile(const std::string& filename) {
    DatasetSplit split;
    
    // open the file for reading
    std::ifstream file(filename);
    
    // check if open was successful
    if (!file.is_open()) {
        throw std::runtime_error("Error: Could not open split file");
    }

    std::string seq_name; // first word in the line is the name of folder
    std::string set_type; // second is the type of set

    // read each line of the file and split the words
    // categorize them into vectors based on set type
    // >> operator reads until whitespace and stores string in variable
    while (file >> seq_name >> set_type) {
        if (set_type == "train") {
            split.train.push_back(seq_name);
        }
        else if (set_type == "validation") {
            split.validation.push_back(seq_name);
        }
        else if (set_type == "test") {
            split.test.push_back(seq_name);
        }
    }

    file.close();
    return split;
}