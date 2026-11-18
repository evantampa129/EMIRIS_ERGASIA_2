#include "config.hpp"
#include <cmath>
#include <fstream>
#include <limits>
#include <map>
#include <stdexcept>
#include <set>

static int integer(const std::string& value) {
    std::size_t end = 0;
    int parsed = std::stoi(value, &end);
    if (end != value.size()) {
        throw std::invalid_argument("Error: invalid integer: " + value);
    }
    return parsed;
}

static double real(const std::string& value) {
    std::size_t end = 0;
    double parsed = std::stod(value, &end);
    if (end != value.size() || !std::isfinite(parsed)) {
        throw std::invalid_argument("Error: invalid number: " + value);
    }
    return parsed;
}

Config parseArguments(int argc, char** argv) {
    Config c;
    std::map<std::string, std::string*> strings{{"-hp", &c.hp_path},
                                                {"-mir", &c.mir_path},
                                                {"-split", &c.split_file},
                                                {"-set", &c.query_set},
                                                {"-o", &c.output_file}};
    std::map<std::string, int*> ints{{"-vocab", &c.vocab},
                                     {"-S", &c.S},
                                     {"-D", &c.D},
                                     {"-seed", &c.seed},
                                     {"-k", &c.lsh_k},
                                     {"-L", &c.lsh_L},
                                     {"-kproj", &c.hc_kproj},
                                     {"-probes", &c.hc_probes},
                                     {"-nbits", &c.ivfpq_nbits},
                                     {"-C", &c.candidates},
                                     {"-T", &c.ransac_trials}};
    std::map<std::string, double*> reals{{"-rho", &c.rho}, {"-tau", &c.tau}, {"-p", &c.confidence}};
    std::set<std::string> seen;
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (!seen.insert(arg).second) {
            throw std::invalid_argument("Error: repeated option: " + arg);
        }
        if (arg == "-exact" || arg == "-lsh" || arg == "-hypercube" || arg == "-ivfflat" || arg == "-ivfpq") {
            if (!c.method.empty()) {
                throw std::invalid_argument("Error: choose exactly one search method");
            }
            c.method = arg;
            continue;
        }
        if (!strings.count(arg) && !ints.count(arg) && !reals.count(arg) && arg != "-w" && arg != "-M" &&
            arg != "-nprobe" && arg != "-kclusters" && arg != "-adaptive") {
            throw std::invalid_argument("Error: unknown option: " + arg);
        }
        if (++i >= argc) {
            throw std::invalid_argument("Error: missing value for " + arg);
        }
        std::string value = argv[i];
        if (strings.count(arg)) {
            *strings[arg] = value;
        }
        else if (ints.count(arg)) {
            *ints[arg] = integer(value);
        }
        else if (reals.count(arg)) {
            *reals[arg] = real(value);
        }
        else if (arg == "-w") {
            c.lsh_w = c.hc_w = real(value);
        }
        else if (arg == "-M") {
            c.hc_M = c.ivfpq_M = integer(value);
        }
        else if (arg == "-nprobe") {
            c.ivfflat_nprobe = c.ivfpq_nprobe = integer(value);
        }
        else if (arg == "-kclusters") {
            c.ivfflat_kclusters = c.ivfpq_kclusters = integer(value);
            c.kclusters_given = true;
        }
        else if (arg == "-adaptive") {
            int flag = integer(value);
            if (flag != 0 && flag != 1) {
                throw std::invalid_argument("Error: adaptive must be 0 or 1");
            }
            c.adaptive = flag == 1;
        }
    }
    return c;
}

void validateVerificationConfig(const Config& c) {
    if (c.hp_path.empty() || c.split_file.empty() || c.output_file.empty() || c.method.empty()) {
        throw std::invalid_argument("Error: -hp, -split, -o and one search method are required");
    }
    if (c.vocab <= 0 || c.S <= 0 || c.D < 0 || c.D > 25000 || c.seed < 0) {
        throw std::invalid_argument("Error: vocab/S must be positive, 0<=D<=25000 and seed>=0");
    }
    if (c.D > 0 && c.mir_path.empty()) {
        throw std::invalid_argument("Error: -mir is required for D>0");
    }
    if (c.query_set != "validation" && c.query_set != "test") {
        throw std::invalid_argument("Error: set must be validation or test");
    }
    if (c.candidates < 10 || c.ransac_trials <= 0 || !std::isfinite(c.rho) || c.rho <= 0 || c.rho >= 1 ||
        !std::isfinite(c.tau) || c.tau <= 0 || !std::isfinite(c.confidence) || c.confidence <= 0 || c.confidence >= 1) {
        throw std::invalid_argument("Error: C>=10, T>0, tau>0, 0<rho<1 and 0<p<1 are required");
    }
    if ((c.method == "-ivfflat" || c.method == "-ivfpq") && (!c.kclusters_given || c.ivfflat_kclusters <= 0)) {
        throw std::invalid_argument("Error: IVF methods require an explicit positive -kclusters");
    }
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