#include "index.hpp"
#include "exact.hpp"
#include "hypercube.hpp"
#include "ivfflat.hpp"
#include "ivfpq.hpp"
#include "lsh.hpp"
#include <stdexcept>

// config.method keeps the flag as it was typed, with the leading dash
std::unique_ptr<SearchIndex> createIndex(const Config& config) {
    unsigned seed = static_cast<unsigned>(config.seed);

    if (config.method == "-exact") {
        return std::make_unique<ExactSearch>();
    }
    if (config.method == "-lsh") {
        return std::make_unique<LSH>(config.lsh_k, config.lsh_L, config.lsh_w, seed);
    }
    if (config.method == "-hypercube") {
        return std::make_unique<Hypercube>(config.hc_kproj, config.hc_w, config.hc_M, config.hc_probes, seed);
    }
    if (config.method == "-ivfflat") {
        return std::make_unique<IVFFlat>(config.ivfflat_kclusters, config.ivfflat_nprobe, seed);
    }
    if (config.method == "-ivfpq") {
        return std::make_unique<IVFPQ>(config.ivfpq_kclusters, config.ivfpq_nprobe,
                                       config.ivfpq_M, config.ivfpq_nbits, seed);
    }
    throw std::invalid_argument("Error: choose one of -exact, -lsh, -hypercube, -ivfflat, -ivfpq");
}
