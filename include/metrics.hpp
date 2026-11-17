#pragma once

#include <string>
#include <vector>

// every query has exactly five relevant images, I2..I6 of its own sequence
constexpr int RELEVANT_PER_QUERY = 5;

// an image is relevant to a query if its id is "<sequence_id>/2" through "<sequence_id>/6"
// mirflickr ids (e.g. im123.jpg) are never relevant
bool isRelevant(const std::string& image_id, const std::string& sequence_id);

// Recall@n = |Top_n(Q) ∩ R(Q)| / 5, missing positions (less than n results) count as not relevant
double recallAtN(const std::vector<std::string>& ranked_ids, const std::string& sequence_id, int n);

// AP@10 = (1/5) * sum j / r_j over the relevant images found at positions r_1 < ... < r_m of the top 10
double averagePrecisionAt10(const std::vector<std::string>& ranked_ids, const std::string& sequence_id);

// mean of a list of values, 0 for an empty list (used for mean recall, mAP@10 and mean time)
double mean(const std::vector<double>& values);
