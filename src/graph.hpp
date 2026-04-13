#pragma once

#include <vector>
#include <cassert>
#include <algorithm>
#include <ranges>
#include <random>
#include <map>
#include <set>
// #include <bits/stdc++.h>
#include <bitset>
#include <bit>

#include "../include/permutations.h"


template<size_t GRAPH_SIZE>
class Graph{
private:

public:
    // Maps vertices to their edges, where an edge between
    // two vertices exists if graph[v_1] << v_2 == 1.
    // i.e. adjMatrix[vertex] -> 00100101, where 0 <= vertex < GRAPH_SIZE,
    // and the 0's in 00100101 mean no edge, and the 1's mean an edge
    const std::vector<std::bitset<GRAPH_SIZE>> adjMatrix;
    std::vector<int> values; // values[i] holds the value at vertex i


    inline Graph(std::vector<std::bitset<GRAPH_SIZE>>& graph) : adjMatrix(graph) {
        this->resetValues();
    }

    // Gets the next lexicographic pinnacle set that
    // maintains strictly increasing ordering.
    static bool getNextPinnacleSet(std::vector<int>& p){
        if(p.size() < 2){
            return false;
        }

        int k = static_cast<int>(p.size());
        int targetIdx = -1;

        for(int i = k - 2; i >= 0; --i){
            int ceiling = static_cast<int>(GRAPH_SIZE) - (k - 1 - i);
            if(p[i] < ceiling){
                targetIdx = i;
                break;
            }
        }

        if(targetIdx == -1){
            return false;
        }

        p[targetIdx]++;

        for(int j = targetIdx + 1; j < k - 1; ++j){
            p[j] = p[j - 1] + 1;
        }

        return true;
    }

    bool isValidLabeling(const std::vector<int>& pinnacleSet) const {
        if(pinnacleSet.empty() || pinnacleSet.back() != static_cast<int>(GRAPH_SIZE)){
            return false;
        }

        // Map pinnacle values to a bitset for O(1) lookup
        // Since pinnacleSet contains labels (1-indexed), we map them to 0-indexed bits
        std::bitset<GRAPH_SIZE> isPinnacleValue;
        for(int val : pinnacleSet){
            isPinnacleValue.set(val - 1);
        }

        for(size_t i = 0; i < GRAPH_SIZE; ++i){
            int currentLabel = values[i];

            // iterate only over existing edges
            unsigned long long mask = adjMatrix[i].to_ullong();

            // A vertex is a pinnacle if it has neighbors AND
            // none of those neighbors have a higher label.
            bool hasNeighbors = (mask > 0);
            bool hasGreaterNeighbor = false;

            while(mask > 0){
                // Get the index of the next set bit (neighbor)
                int j = std::countr_zero(mask); // ignore LSP error, this project uses C++20 so this works

                if(values[j] > currentLabel){
                    hasGreaterNeighbor = true;
                    break;
                }

                // Clear the lowest set bit to move to the next neighbor
                mask &= (mask - 1);
            }

            bool actsAsPinnacle = hasNeighbors && !hasGreaterNeighbor;
            bool shouldBePinnacle = isPinnacleValue.test(currentLabel - 1);

            if(actsAsPinnacle != shouldBePinnacle){
                return false;
            }
        }

        return true;
    }

    std::vector<std::vector<int>> getAdmissablePinnacleSets(){
        return {{}};
    }

    // determines all independent sets of a given size
    std::vector<std::vector<int>> getIndependentSets(int size){
        return {{}};
    }

    void resetValues(){
        this->values = std::vector<int>(GRAPH_SIZE);
        for(int i = 0; i < GRAPH_SIZE; ++i) {
            this->values[i] = i + 1;
        }
    }

    void setValues(const std::vector<int>& vals){
        std::copy(vals.begin(), vals.end(), this->values.begin());
    }

    static std::vector<std::bitset<GRAPH_SIZE>> makeCompleteGraph(const std::vector<int>& pinnacleSet){
        std::vector<std::bitset<GRAPH_SIZE>> res(GRAPH_SIZE);

        // Using a bitset for pinnacle lookup (O(1))
        std::bitset<GRAPH_SIZE + 1> isPinnacle;
        for(int p : pinnacleSet){
            if(p > 0 && p <= (int)GRAPH_SIZE){
                isPinnacle.set(p);
            }
        }

        unsigned long long completeMask = (GRAPH_SIZE == 64) ? ~0ULL : (1ULL << GRAPH_SIZE) - 1;

        for(size_t i = 0; i < GRAPH_SIZE; ++i){
            int vertexValue = (int)i + 1; // Direct 1-based mapping

            if(isPinnacle.test(vertexValue)){
                // Pinnacle: connects to indices < i (smaller labels)
                unsigned long long smallerValuesMask = (1ULL << i) - 1;
                res[i] = std::bitset<GRAPH_SIZE>(smallerValuesMask);
            } else{
                // Non-pinnacle: connects to all except self
                unsigned long long selfMask = (1ULL << i);
                res[i] = std::bitset<GRAPH_SIZE>(completeMask ^ selfMask);
            }
        }

        return res;
    }
};
