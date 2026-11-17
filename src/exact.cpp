#include "exact.hpp"
#include <algorithm>
#include <queue>
#include <stdexcept>

// keeps the k closest candidates with a max heap of size k
// the top of the heap is the worst of the current k, so it is replaced when a closer one is found
// time O(c*K + c*log k) for c candidates of dimension K, extra memory O(k)
std::vector<Neighbor> rankCandidates(const Matrix& database, const std::vector<float>& query,
                                     const std::vector<int>& candidates, int k) {
    std::vector<Neighbor> result;
    if (k <= 0) {
        return result;
    }

    std::priority_queue<Neighbor, std::vector<Neighbor>, decltype(&closerNeighbor)> heap(&closerNeighbor);
    for (int idx : candidates) {
        Neighbor current{idx, squaredDistance(database[idx], query)};

        if (static_cast<int>(heap.size()) < k) {
            heap.push(current);
        }
        else if (closerNeighbor(current, heap.top())) {
            heap.pop();
            heap.push(current);
        }
    }

    // empty the heap, it gives the worst first, so reverse to get increasing distance
    while (!heap.empty()) {
        result.push_back(heap.top());
        heap.pop();
    }
    std::reverse(result.begin(), result.end());
    return result;
}

// every database vector is a candidate
std::vector<Neighbor> exactSearch(const Matrix& database, const std::vector<float>& query, int k) {
    std::vector<int> all(database.size());
    for (size_t i = 0; i < database.size(); ++i) {
        all[i] = static_cast<int>(i);
    }
    return rankCandidates(database, query, all, k);
}

// exact search only needs a pointer to the database
void ExactSearch::build(const Matrix& database) {
    data = &database;
}

std::vector<Neighbor> ExactSearch::search(const std::vector<float>& query, int k) const {
    if (data == nullptr) {
        throw std::logic_error("Error: search called before build");
    }
    return exactSearch(*data, query, k);
}
