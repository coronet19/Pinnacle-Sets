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

#include "permutations.h"


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


    inline Graph(std::vector<std::bitset<GRAPH_SIZE>>& graph){
        std::copy(graph.begin(), graph.end(), adjMatrix.begin());

        // Initialize graph values (1-based for the math, 0-indexed for the vector)
        this->values = std::vector<int>(GRAPH_SIZE);
        for(int i = 0; i < GRAPH_SIZE; ++i) {
            values[i] = i + 1;
        }
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

    // checks if pinnacleSet is a valid pinnacle set for graph
    bool isValidLabeling(const std::vector<int>& pinnacleSet){
        if(pinnacleSet.back() != static_cast<int>(GRAPH_SIZE)) return false;
        if(!std::is_sorted(pinnacleSet.begin(), pinnacleSet.end())) return false;

        for(int vertexIdx = 0; vertexIdx < static_caste<int>(GRAPH_SIZE); ++vertexIdx){
            std::bitset<GRAPH_SIZE> neighbors = this->adjMatrix[vertexIdx];
            int vertexValue = this->values[vertexIdx];
            bool shouldBePinnacle = std::binary_search(pinnacleSet.begin(), pinnacleSet.end(), vertexValue);
            bool actsAsPinnacle = neighbors.none();

            for(int edgeIdx = 0; edgeIdx < GRAPH_SIZE; ++edgeIdx){
                if(graph[vertexIdx].test(edgeIdx)){ // if edge exists
                    if(vertexValue <= values[edgeIdx]){
                        actsAsPinnacle = false;
                        break;
                    }
                }
            }

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
};
