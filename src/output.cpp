#include "output.hpp"
#include <fstream>
#include <stdexcept>

// one block per query, missing ranks (less than 10 results) are not printed
void writeResults(std::ostream& out, const std::string& method_name,
                  const std::vector<QueryResult>& results, const EvaluationSummary& summary) {
    out << method_name << "\n";

    for (const QueryResult& r : results) {
        out << "Query: " << r.sequence_id << "\n";
        for (size_t i = 0; i < r.ranked_ids.size(); ++i) {
            out << "Rank-" << (i + 1) << ": " << r.ranked_ids[i] << "\n";
        }
        out << "Recall@5: " << r.recall5 << "\n";
        out << "Recall@10: " << r.recall10 << "\n";
        out << "AP@10: " << r.ap10 << "\n";
        out << "\n";
    }

    out << "Mean Recall@5: " << summary.mean_recall5 << "\n";
    out << "Mean Recall@10: " << summary.mean_recall10 << "\n";
    out << "mAP@10: " << summary.map10 << "\n";
    out << "Average query time: " << summary.avg_time_ms << "\n";
}

// open the output file and write to it
void writeResultsFile(const std::string& filename, const std::string& method_name,
                      const std::vector<QueryResult>& results, const EvaluationSummary& summary) {
    std::ofstream file(filename);
    if (!file.is_open()) {
        throw std::runtime_error("Error: Could not open output file");
    }
    writeResults(file, method_name, results, summary);
}

// one line per group of queries
void printSummary(std::ostream& out, const std::string& method_name, const std::vector<QueryResult>& results) {
    const std::string groups[3][2] = {{"", "all"}, {"i_", "illumination"}, {"v_", "viewpoint"}};

    out << method_name << "\n";
    for (const auto& group : groups) {
        EvaluationSummary s = summarize(results, group[0]);
        out << "  " << group[1] << " (" << s.queries << " queries)"
            << ": Recall@5 " << s.mean_recall5
            << ", Recall@10 " << s.mean_recall10
            << ", mAP@10 " << s.map10
            << ", time " << s.avg_time_ms << " ms"
            << ", <10 results " << s.short_queries << "\n";
    }
}
