#include "cached_preprocess.hpp"
#include "config.hpp"
#include "evaluation.hpp"
#include "index.hpp"
#include "output.hpp"
#include <chrono>
#include <exception>
#include <filesystem>
#include <iostream>
#include <stdexcept>

namespace fs = std::filesystem;

// folder of the cached vocabulary and vectors, relative to the folder the program runs from
static const std::string CACHE_DIR = "cache";

// parameters without a default value must be given on the command line
static void checkConfig(const Config& config) {
    if (config.hp_path.empty() || config.split_file.empty() || config.output_file.empty()) {
        throw std::invalid_argument("Error: -hp, -split and -o are required");
    }
    if (config.D > 0 && config.mir_path.empty()) {
        throw std::invalid_argument("Error: -mir is required when -D is larger than 0");
    }
    if (config.vocab <= 0 || config.S <= 0 || config.D < 0) {
        throw std::invalid_argument("Error: -vocab and -S must be positive and -D must not be negative");
    }
    if (config.query_set != "validation" && config.query_set != "test") {
        throw std::invalid_argument("Error: -set must be validation or test");
    }
}

// the datasets can be anywhere, the paths are checked before the long preprocessing
// a folder that only contains the extracted folder of the archive (e.g. mirflickr/mirflickr) is also accepted
static void checkPaths(Config& config, const DatasetSplit& split) {
    if (split.train.empty() || (config.query_set == "validation" ? split.validation : split.test).empty()) {
        throw std::invalid_argument("Error: the split file has no train or no " + config.query_set + " sequences");
    }

    const std::string& sequence = split.train[0];
    if (!fs::exists(fs::path(config.hp_path) / sequence) && fs::exists(fs::path(config.hp_path) / "hpatches-sequences-release" / sequence)) {
        config.hp_path = (fs::path(config.hp_path) / "hpatches-sequences-release").string();
    }
    if (!fs::exists(fs::path(config.hp_path) / sequence / "1.ppm")) {
        throw std::invalid_argument("Error: " + (fs::path(config.hp_path) / sequence / "1.ppm").string() + " not found, check -hp and -split");
    }

    if (config.D > 0) {
        if (!fs::exists(fs::path(config.mir_path) / "im1.jpg") && fs::exists(fs::path(config.mir_path) / "mirflickr" / "im1.jpg")) {
            config.mir_path = (fs::path(config.mir_path) / "mirflickr").string();
        }
        if (!fs::exists(fs::path(config.mir_path) / "im1.jpg")) {
            throw std::invalid_argument("Error: " + (fs::path(config.mir_path) / "im1.jpg").string() + " not found, check -mir");
        }
    }
}

int main(int argc, char** argv) {
    try {
        Config config = parseArguments(argc, argv);
        checkConfig(config);

        // the method is created first, so a wrong method is reported before the long preprocessing
        std::unique_ptr<SearchIndex> index = createIndex(config);

        // train sequences build the vocabulary, the chosen set gives the queries and their relevant images
        DatasetSplit split = readSplitFile(config.split_file);
        checkPaths(config, split);
        Matrix centers = loadOrBuildVocabulary(config, split, CACHE_DIR);

        VectorSet queries;
        VectorSet database;
        loadOrBuildVectors(config, split, centers, CACHE_DIR, queries, database);
        std::cout << "Queries: " << queries.ids.size() << ", database: " << database.ids.size() << " images\n";

        auto start = std::chrono::steady_clock::now();
        index->build(database.vectors);
        std::cerr << "index: " << index->name() << " built in "
                  << std::chrono::duration<double>(std::chrono::steady_clock::now() - start).count() << " s\n";

        std::vector<QueryResult> results = runQueries(*index, database, queries);
        writeResultsFile(config.output_file, index->name(), results, summarize(results));
        printSummary(std::cout, index->name(), results);
    }
    catch (const std::exception& e) {
        std::cerr << e.what() << "\n";
        return 1;
    }
    return 0;
}
