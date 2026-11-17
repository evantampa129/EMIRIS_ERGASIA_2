#include "metrics.hpp"
#include <algorithm>

// the five relevant images are images 2 through 6 of the query sequence
bool isRelevant(const std::string& image_id, const std::string& sequence_id) {
    for (int image_number = 2; image_number <= 6; ++image_number) {
        if (image_id == sequence_id + "/" + std::to_string(image_number)) {
            return true;
        }
    }
    return false;
}

// count the relevant images in the first n positions
double recallAtN(const std::vector<std::string>& ranked_ids, const std::string& sequence_id, int n) {
    int limit = std::min(n, static_cast<int>(ranked_ids.size()));
    int found = 0;
    for (int i = 0; i < limit; ++i) {
        if (isRelevant(ranked_ids[i], sequence_id)) {
            ++found;
        }
    }
    return static_cast<double>(found) / RELEVANT_PER_QUERY;
}

// the j-th relevant image found at rank r_j adds j / r_j, ranks start from 1
double averagePrecisionAt10(const std::vector<std::string>& ranked_ids, const std::string& sequence_id) {
    int limit = std::min(10, static_cast<int>(ranked_ids.size()));
    int found = 0;
    double sum = 0.0;
    for (int i = 0; i < limit; ++i) {
        if (isRelevant(ranked_ids[i], sequence_id)) {
            ++found;
            sum += static_cast<double>(found) / (i + 1);
        }
    }
    return sum / RELEVANT_PER_QUERY;
}

// arithmetic mean
double mean(const std::vector<double>& values) {
    if (values.empty()) {
        return 0.0;
    }
    double sum = 0.0;
    for (double v : values) {
        sum += v;
    }
    return sum / values.size();
}
