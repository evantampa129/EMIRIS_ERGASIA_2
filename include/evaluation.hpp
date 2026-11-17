#pragma once

#include "index.hpp"
#include <string>
#include <vector>

// vectors together with the id of each one, this is what the data pipeline gives to the search part
// database ids: "<sequence_id>/<number>" for hpatches (e.g. v_bird/2), file name for mirflickr (e.g. im123.jpg)
// query ids: the sequence id (e.g. v_bird), its relevant images are database ids 2 through 6
struct VectorSet {
    Matrix vectors;                // one normalized bag of visual words vector per image
    std::vector<std::string> ids;  // ids[i] belongs to vectors[i]
};

// results of one query
struct QueryResult {
    std::string sequence_id;              // query sequence (e.g. v_bird)
    std::vector<std::string> ranked_ids;  // up to 10 retrieved image ids, best first
    double recall5 = 0.0;                 // Recall@5
    double recall10 = 0.0;                // Recall@10
    double ap10 = 0.0;                    // AP@10
    double time_ms = 0.0;                 // search time of this query in milliseconds
};

// mean values over a set of queries
struct EvaluationSummary {
    int queries = 0;             // number of queries in the summary
    double mean_recall5 = 0.0;   // Mean Recall@5
    double mean_recall10 = 0.0;  // Mean Recall@10
    double map10 = 0.0;          // mAP@10
    double avg_time_ms = 0.0;    // average query time in milliseconds
    int short_queries = 0;       // queries with less than 10 results
};

// runs every query on an already built index and computes its metrics
// only the index.search() call is timed, not the index construction or the vectorization
std::vector<QueryResult> runQueries(const SearchIndex& index, const VectorSet& database,
                                    const VectorSet& queries, int k = 10);

// mean metrics of the queries whose sequence id starts with prefix ("" for all, "i_" illumination, "v_" viewpoint)
EvaluationSummary summarize(const std::vector<QueryResult>& results, const std::string& prefix = "");
