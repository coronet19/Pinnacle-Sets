#include "../include/CompleteGraph.h"


// Gets the next lexicographic pinnacle set that
// maintains strictly increasing ordering
bool CompleteGraph::getNextPinnacleSet(std::vector<int>& p) {
    if (p.size() < 2) {
        // If there's only one element, and it must be 'size',
        // there are no other combinations to generate.
        return false;
    }

    int k = static_cast<int>(p.size());

    // The last element p[k-1] is fixed to this->size.
    // The element at index i can at most be: (size - (k - 1 - i))
    // Example: size=10, k=3. Set is {x, y, 10}.
    // y can be at most 9. x can be at most 8.

    int targetIdx = -1;

    // 1. Find the rightmost element that can be incremented.
    // We start from k-2 because p[k-1] (the last element) is fixed.
    for (int i = k - 2; i >= 0; --i) {
        int ceiling = this->size - (k - 1 - i);
        if (p[i] < ceiling) {
            targetIdx = i;
            break;
        }
    }

    // If no such element exists, we are at the lexicographical maximum.
    if (targetIdx == -1) {
        return false;
    }

    // 2. Increment the target element.
    p[targetIdx]++;

    // 3. Reset all elements to the right (up to k-2) to be strictly increasing.
    // This ensures we get the smallest possible sequence following our increment.
    for (int j = targetIdx + 1; j < k - 1; ++j) {
        p[j] = p[j - 1] + 1;
    }

    return true;
}

// get all admissable pinnacle sets for the graph structure edgeMap
std::vector<std::vector<int>> CompleteGraph::getAdmissablePinnacleSets(){
    std::vector<std::vector<int>> validLabelings;

    // 1. Ensure graph and pinnacles are sorted to start at the first lexicographical permutation
    std::sort(this->graph.begin(), this->graph.end());
    std::sort(this->pinnacles.begin(), this->pinnacles.end());

    // 2. Iterate through EVERY combination of the pinnacles
    do{
        // 3. Iterate through EVERY permutation of the labels (1, 2, 3, ..., size)
        do {
            // 4. Check if the current permutation of graph values
            // makes the pinnacles set valid for this edgeMap
            if (this->isValidLabeling(this->pinnacles)) {
                validLabelings.push_back(this->pinnacles);
            }
        } while (std::next_permutation(this->graph.begin(), this->graph.end()));
    } while(CompleteGraph::getNextPinnacleSet(this->pinnacles));


    return validLabelings;
}

bool CompleteGraph::isValidLabeling(const std::vector<int> pinnacleSet){
    for (const auto& [vertexIdx, neighbors] : this->edgeMap) {
        int vertexValue = this->graph[vertexIdx];

        // 1. Check if this specific vertex value is in our "target" pinnacle set
        // Use std::binary_search since pinnacleSet is sorted (much faster than std::find)
        bool shouldBePinnacle = std::binary_search(pinnacleSet.begin(), pinnacleSet.end(), vertexValue);

        // 2. Determine if it ACTUALLY acts as a pinnacle in the current graph
        bool actsAsPinnacle = !neighbors.empty(); // An isolated vertex usually isn't a pinnacle

        for (int edgeIdx : neighbors) {
            if (vertexValue <= this->graph[edgeIdx]) {
                actsAsPinnacle = false;
                break;
            }
        }

        // 3. If the graph structure doesn't match the set we're testing, it's invalid
        if (actsAsPinnacle != shouldBePinnacle) {
            return false;
        }
    }
    return true;
}
