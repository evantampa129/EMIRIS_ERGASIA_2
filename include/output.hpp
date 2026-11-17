#pragma once

#include "evaluation.hpp"
#include <ostream>
#include <string>
#include <vector>

// writes the results in the format of the assignment:
// method name, then for every query its top 10 ranks and metrics, then the mean metrics
void writeResults(std::ostream& out, const std::string& method_name,
                  const std::vector<QueryResult>& results, const EvaluationSummary& summary);

// same as writeResults but opens the given file, throws std::runtime_error if it cannot be opened
void writeResultsFile(const std::string& filename, const std::string& method_name,
                      const std::vector<QueryResult>& results, const EvaluationSummary& summary);

// short report for the terminal, all queries and separately illumination and viewpoint
// includes the number of queries with less than 10 results, needed for the experimental study
void printSummary(std::ostream& out, const std::string& method_name, const std::vector<QueryResult>& results);
