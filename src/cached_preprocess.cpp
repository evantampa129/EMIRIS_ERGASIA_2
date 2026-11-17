#include "cached_preprocess.hpp"
#include "preprocess.hpp"
#include "vector_cache.hpp"
#include <algorithm>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iostream>
#include <sstream>

namespace fs = std::filesystem;

// K, S, seed and a hash of the split, every cache file name starts with this
static std::string cacheKey(const Config& config, const DatasetSplit& split) {
    std::string names;
    for (const std::vector<std::string>* part : {&split.train, &split.validation, &split.test}) {
        for (const std::string& name : *part) {
            names += name + ",";
        }
        names += ";";
    }

    std::ostringstream key;
    key << "K" << config.vocab << "_S" << config.S << "_seed" << config.seed
        << "_split" << std::hex << std::hash<std::string>{}(names);
    return key.str();
}

static std::string cacheFile(const std::string& cache_dir, const std::string& name) {
    return (fs::path(cache_dir) / (name + ".bin")).string();
}

// the preprocessing gives {id, vector} pairs, the search part expects the vectors and the ids separately
static VectorSet toVectorSet(const std::vector<ImageVector>& images) {
    VectorSet set;
    set.vectors.reserve(images.size());
    set.ids.reserve(images.size());
    for (const ImageVector& image : images) {
        set.vectors.push_back(image.vec);
        set.ids.push_back(image.id);
    }
    return set;
}

Matrix loadOrBuildVocabulary(const Config& config, const DatasetSplit& split, const std::string& cache_dir) {
    std::string file = cacheFile(cache_dir, cacheKey(config, split) + "_vocab");

    VectorSet cached;
    if (loadVectorSet(file, cached)) {
        std::cout << "Vocabulary loaded from " << file << "\n";
        return cached.vectors;
    }

    VectorSet vocab;
    vocab.vectors = buildVocabulary(config.hp_path, split.train, config.vocab, config.S, config.seed);
    // the centers have no ids, they are only numbered so the file format stays the same
    for (size_t c = 0; c < vocab.vectors.size(); ++c) {
        vocab.ids.push_back(std::to_string(c));
    }
    saveVectorSet(file, vocab);
    return vocab.vectors;
}

void loadOrBuildVectors(const Config& config, const DatasetSplit& split, const Matrix& centers,
                        const std::string& cache_dir, VectorSet& queries, VectorSet& database) {
    std::string key = cacheKey(config, split);
    const std::vector<std::string>& active_seqs = (config.query_set == "test") ? split.test : split.validation;

    // hpatches queries (image 1) and their relevant images (2 to 6) of the chosen set
    std::string queries_file = cacheFile(cache_dir, key + "_" + config.query_set + "_queries");
    std::string hpatches_file = cacheFile(cache_dir, key + "_" + config.query_set + "_database");
    VectorSet hpatches;
    if (loadVectorSet(queries_file, queries) && loadVectorSet(hpatches_file, hpatches)) {
        std::cout << "HPatches vectors of the " << config.query_set << " set loaded from the cache\n";
    }
    else {
        // D = 0, so only the hpatches part of the preprocessing runs
        Config hpatches_config = config;
        hpatches_config.D = 0;
        std::vector<ImageVector> query_images;
        std::vector<ImageVector> database_images;
        buildQueriesAndDatabase(hpatches_config, active_seqs, centers, query_images, database_images);
        queries = toVectorSet(query_images);
        hpatches = toVectorSet(database_images);
        saveVectorSet(queries_file, queries);
        saveVectorSet(hpatches_file, hpatches);
    }

    // mirflickr distractors, computed again only if the cache has less than D images
    // and does not already hold every valid image of the folder
    std::string mirflickr_file = cacheFile(cache_dir, key + "_mirflickr");
    std::string complete_file = cacheFile(cache_dir, key + "_mirflickr_complete");
    VectorSet mirflickr;
    bool cached = loadVectorSet(mirflickr_file, mirflickr);
    bool complete = cached && fs::exists(complete_file);
    if (config.D > 0 && (!cached || (mirflickr.ids.size() < static_cast<size_t>(config.D) && !complete))) {
        // no hpatches sequences, so only the mirflickr part of the preprocessing runs
        std::vector<ImageVector> unused_queries;
        std::vector<ImageVector> mirflickr_images;
        buildQueriesAndDatabase(config, {}, centers, unused_queries, mirflickr_images);
        mirflickr = toVectorSet(mirflickr_images);
        saveVectorSet(mirflickr_file, mirflickr);
        fs::remove(complete_file);

        // fewer than D images means that every file was read, the marker keeps the next runs from reading them again
        if (mirflickr.ids.size() < static_cast<size_t>(config.D)) {
            std::ofstream(complete_file).put('\n');
        }
    }
    else if (config.D > 0) {
        std::cout << "MIRFlickr vectors loaded from the cache (" << mirflickr.ids.size() << " images, using the first "
                  << std::min(config.D, static_cast<int>(mirflickr.ids.size())) << ")\n";
    }

    // database: hpatches images first, then the first D mirflickr images, or all of them if there are fewer valid ones
    database = hpatches;
    int distractors = std::min(config.D, static_cast<int>(mirflickr.ids.size()));
    std::cout << "MIRFlickr distractors in the database: " << distractors << "\n";
    for (int i = 0; i < distractors; ++i) {
        database.vectors.push_back(mirflickr.vectors[i]);
        database.ids.push_back(mirflickr.ids[i]);
    }
}
