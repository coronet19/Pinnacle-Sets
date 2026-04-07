#pragma once

#include <vector>
#include <cassert>
#include <algorithm>
#include <ranges>
#include <random>
#include <map>
#include <set>

#include "permutations.h"


class CompleteGraph{
private:

public:
    // Maps vertices to their edges, where edges are the indices of the graph vector.
    // i.e. edgeMap[vertex] -> { i_0, i_1, ... , i_n }, where 0 <= vertex < size
    inline static std::vector<std::vector<int>> edgeMap;
    std::vector<int> pinnacles;
    std::vector<int> graph;
    int size;

    inline CompleteGraph(int size, std::vector<int> pinnacleSet){
        this->size = size;
        this->pinnacles = pinnacleSet;
        this->graph = std::vector<int>(size);

        edgeMap.clear();
        edgeMap.resize(size);

        std::sort(this->pinnacles.begin(), this->pinnacles.end());

        // Initialize graph values (1-based for the math, 0-indexed for the vector)
        for(int i = 0; i < size; ++i) {
            graph[i] = i + 1;
        }

        // Convert pinnacleSet to a set for O(log n) lookups
        std::set<int> pinLookUp(pinnacles.begin(), pinnacles.end());

        for(int i = 0; i < size; ++i) {
            int vertexValue = graph[i];
            bool isPinnacle = pinLookUp.count(vertexValue);

            for(int j = 0; j < size; ++j) {
                if(i == j) continue;
                int neighborValue = graph[j];

                if(isPinnacle) {
                    // RULE: If I am a pinnacle, I can only have edges to vertices SMALLER than me.
                    if(neighborValue < vertexValue) {
                        this->edgeMap[i].push_back(j);
                    }
                } else {
                    // If I am NOT a pinnacle, I keep all edges
                    this->edgeMap[i].push_back(j);
                }
            }
        }
    }

    // Gets the next lexicographic pinnacle set that
    // maintains strictly increasing ordering
    bool getNextPinnacleSet(std::vector<int>& p);
    bool isValidLabeling(const std::vector<int> pinnacleSet);
    std::vector<std::vector<int>> getAdmissablePinnacleSets();
};
