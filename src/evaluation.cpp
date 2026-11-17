#include "evaluation.hpp"
#include "metrics.hpp"
#include <chrono>
#include <stdexcept>

// for each query, time only the search and then turn the result indices into image ids
std::vector<QueryResult> runQueries(const SearchIndex& index, const VectorSet& database,
                                    const VectorSet& queries, int k) {
    if (database.vectors.size() != database.ids.size() || queries.vectors.size() != queries.ids.size()) {
        throw std::invalid_argument("Error: every vector must have exactly one id");
    }

    std::vector<QueryResult> results;
    results.reserve(queries.vectors.size());

    for (size_t q = 0; q < queries.vectors.size(); ++q) {
        // steady_clock is monotonic, so it is not affected by changes of the system time
        auto start = std::chrono::steady_clock::now();
        std::vector<Neighbor> neighbors = index.search(queries.vectors[q], k);
        auto end = std::chrono::steady_clock::now();

        QueryResult result;
        result.sequence_id = queries.ids[q];
        result.time_ms = std::chrono::duration<double, std::milli>(end - start).count();
        for (const Neighbor& n : neighbors) {
            result.ranked_ids.push_back(database.ids[n.index]);
        }

        result.recall5 = recallAtN(result.ranked_ids, result.sequence_id, 5);
        result.recall10 = recallAtN(result.ranked_ids, result.sequence_id, 10);
        result.ap10 = averagePrecisionAt10(result.ranked_ids, result.sequence_id);
        results.push_back(result);
    }
    return results;
}

// keep the queries with the wanted prefix and average their metrics
EvaluationSummary summarize(const std::vector<QueryResult>& results, const std::string& prefix) {
    std::vector<double> recall5, recall10, ap10, times;
    EvaluationSummary summary;

    for (const QueryResult& r : results) {
        if (r.sequence_id.compare(0, prefix.size(), prefix) != 0) {
            continue;
        }
        recall5.push_back(r.recall5);
        recall10.push_back(r.recall10);
        ap10.push_back(r.ap10);
        times.push_back(r.time_ms);
        if (r.ranked_ids.size() < 10) {
            ++summary.short_queries;
        }
    }

    summary.queries = static_cast<int>(ap10.size());
    summary.mean_recall5 = mean(recall5);
    summary.mean_recall10 = mean(recall10);
    summary.map10 = mean(ap10);
    summary.avg_time_ms = mean(times);
    return summary;
}
