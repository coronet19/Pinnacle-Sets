#pragma once

#include <vector>
#include <cstdint> // defines uint8_t
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
    std::map<int, std::vector<int>> edgeMap;
    std::vector<int> pinnacles;
    std::vector<int> graph;
    int size;

    inline CompleteGraph(int size, std::vector<int> pinnacleSet){
        // this->size = size;
        // this->pinnacles = pinnacleSet;
        // this->graph = std::vector<int>(size);

        // sort(this->pinnacles.begin(), this->pinnacles.end());

        // assert(pinnacleSet.size() < size);
        // if(!this->pinnacles.empty()){
        //     assert(this->pinnacles.back() == size);
        // }

        // for(int i = 0; i < size; ++i){
        //     graph[i] = i + 1;
        // }

        // // determine edgeMap
        // // For each vertex p in the graph, if p is a pinnacle
        // // then remove all edges to vertices v > p
        // int k = 0; // -> pinnacleSet iterator
        // for(int i = 0; i < this->size; ++i){
        //     int p = this->pinnacles[k] - 1;

        //     if(i <= p){ // if i is not a pinnacle
        //         for(int j = 0; j < this->size; ++j){
        //             if(i == j) continue;
        //             this->edgeMap[i].push_back(j);
        //         }
        //     } else{ // if i is a pinnacle
        //         for(int j = 0; j < k; ++j){
        //             if(i == j) continue;
        //             this->edgeMap[i].push_back(j);
        //         }
        //         ++k;
        //     }
        // }
        this->size = size;
        this->pinnacles = pinnacleSet;
        this->graph = std::vector<int>(size);

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
                    // RULE: If I am a pinnacle, I can only have edges
                    // to vertices SMALLER than me.
                    if(neighborValue < vertexValue) {
                        this->edgeMap[i].push_back(j);
                    }
                } else {
                    // If I am NOT a pinnacle, I keep all edges
                    // (Standard Complete Graph behavior)
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
