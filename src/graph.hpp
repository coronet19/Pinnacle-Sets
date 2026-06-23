#pragma once

#include <vector>
#include <cassert>
#include <algorithm>
// #include <ranges>
// #include <random>
// #include <map>
#include <set>
// #include <bits/stdc++.h>
#include <bitset>
#include <bit>
#include <iostream>
#include <fstream>
#include <filesystem>
#include <queue>
#include <thread>
#include <atomic>
#include <mutex>

// #include "../include/permutations.h"


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

    // takes in a base pinnacle set b of size n and returns
    // all pinnacle sets p of size n where p > b
    std::vector<std::vector<int>> getAdmissablePinnacleSets(std::vector<int> pinnacles) {
        std::vector<std::vector<int>> admissable;
        std::sort(pinnacles.begin(), pinnacles.end());

        // Set up all labels [1...GRAPH_SIZE]
        std::vector<int> allLabels(GRAPH_SIZE);
        for(int i = 0; i < (int)GRAPH_SIZE; ++i) allLabels[i] = i + 1;

        do {
            if (pinnacles.empty() || pinnacles.back() != static_cast<int>(GRAPH_SIZE)) continue;

            auto indySets = this->getIndependentSets(pinnacles.size());
            bool possible = false;

            for (const auto& indices : indySets) {
                // Create a list of labels that ARE NOT pinnacles
                std::vector<int> nonPinnacleLabels;
                std::set<int> pSet(pinnacles.begin(), pinnacles.end());
                for(int l : allLabels) if(!pSet.count(l)) nonPinnacleLabels.push_back(l);

                // OPTIMIZATION: Instead of full permutation, try a "Safe" assignment:
                // Put pinnacles on indices, others everywhere else.
                this->resetValues();
                std::vector<bool> isUsed(GRAPH_SIZE, false);
                for(int idx : indices) isUsed[idx] = true;

                // Simple assignment
                int pIdx = 0, npIdx = 0;
                for(int i = 0; i < (int)GRAPH_SIZE; ++i) {
                    if(isUsed[i]) values[i] = pinnacles[pIdx++];
                    else values[i] = nonPinnacleLabels[npIdx++];
                }

                // Now check if this SPECIFIC labeling works
                if (this->isValidLabeling(pinnacles)) {
                    possible = true;
                    break;
                }
            }

            if (possible) admissable.push_back(pinnacles);

        } while (Graph<GRAPH_SIZE>::getNextPinnacleSet(pinnacles));

        return admissable;
    }

    // determines all independent sets of a given size n
    std::vector<std::vector<int>> getIndependentSets(int n) {
        std::vector<std::vector<int>> results;

        // P: Candidate vertices (initially all)
        // R: Current independent set being built
        std::bitset<GRAPH_SIZE> initialP;
        initialP.set();
        std::bitset<GRAPH_SIZE> initialR;

        // Recursive backtracking function
        auto findSets = [&](auto self, std::bitset<GRAPH_SIZE> P, std::bitset<GRAPH_SIZE> R) -> void {
            // Base Case: We reached the target size
            if (R.count() == (size_t)n) {
                std::vector<int> setIndices;
                for (int i = 0; i < (int)GRAPH_SIZE; ++i) {
                    if (R.test(i)) setIndices.push_back(i);
                }
                results.push_back(setIndices);
                return;
            }

            // Pruning: If remaining candidates + current set < target size, stop
            if (R.count() + P.count() < (size_t)n) {
                return;
            }

            // Process candidates
            while (P.any()) {
                // Get the next candidate index using bit-jumping (countr_zero)
                unsigned long long mask = P.to_ullong();
                int v = std::countr_zero(mask); // ignore LSP error

                // 1. Remove v from the candidate pool P
                P.reset(v);

                // 2. Create a new candidate pool for the next branch
                // Crucial: The next candidates must NOT be adjacent to v
                // We do this by ANDing P with the bitwise NOT of v's adjacency row
                std::bitset<GRAPH_SIZE> nextP = P & ~(adjMatrix[v]);

                // 3. Add v to our current result set R and recurse
                std::bitset<GRAPH_SIZE> nextR = R;
                nextR.set(v);

                self(self, nextP, nextR);
            }
        };

        findSets(findSets, initialP, initialR);
        return results;
    }

    void resetValues(){
        this->values = std::vector<int>(GRAPH_SIZE);
        for(size_t i = 0; i < GRAPH_SIZE; ++i) {
            this->values[i] = i + 1;
        }
    }

    void setValues(const std::vector<int>& vals){
        std::copy(vals.begin(), vals.end(), this->values.begin());
    }

    void countHeapPermutations(const std::vector<std::vector<int>>& pinnacles, int size, int& count) {
        if(size == 1){
            for(const auto& p : pinnacles){
                if(this->isValidLabeling(p)){
                    ++count;
                }
            }

            return;
        }

        for(int i = 0; i < size; i++){
            countHeapPermutations(pinnacles, size - 1, count);

            if(size & 1){
                std::swap(this->values[0], this->values[size - 1]);
            } else{
                std::swap(this->values[i], this->values[size - 1]);
            }
        }
    }

    void printGraph(){
        for(int i = 0; i < adjMatrix.size(); ++i){
            for(int j = 0; j < GRAPH_SIZE; ++j){
                std::cout << adjMatrix[i][j];
            }
            std::cout << std::endl;
        }
    }

    void countHeapPermutations(const std::vector<int>& p, int size, int& count) {
        if(size == 1){
            if(this->isValidLabeling(p)){
                ++count;
            }

            return;
        }

        for(int i = 0; i < size; i++){
            countHeapPermutations(p, size - 1, count);

            if(size % 2 == 1){
                std::swap(this->values[0], this->values[size - 1]);
            } else{
                std::swap(this->values[i], this->values[size - 1]);
            }
        }
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

    static void getGraphStats(const std::string& path){
        std::filesystem::path originalPath(path);

        // Get the path without the extension (e.g., "data/graph5c")
        std::filesystem::path newPath = originalPath.parent_path() / originalPath.stem();

        // Append "_stats" and put the original extension back (e.g., "data/graph5c_stats.g6")
        newPath += "_stats";
        newPath += originalPath.extension();

        std::ifstream file(path);
        if(!file.is_open()){
            std::cerr << "Error: Could not open input file at " << path << std::endl;
            exit(1);
        }

        std::ofstream res(newPath.string());
        if(!res.is_open()){
            std::cerr << "Error: Could not create output file at " << newPath.string() << std::endl;
            exit(1);
        }

        const std::vector<std::vector<int>> pinnacleSets = generatePinnacleSets();
        int numGraphsAnalyzed = 0;
        std::string line;
        while(std::getline(file, line)){
            if(line.empty()) continue;

            auto decoded_graph = decodeGraphG6(line);
            Graph<GRAPH_SIZE> g(decoded_graph);
            std::vector<int> stats = Graph::analyzeStats(g, pinnacleSets);

            res << line << ",[" << stats[0] << "," << stats[1] << "," << stats[2] << "]\n";
            // g.printGraph();
            // std::cout << std::endl;

            ++numGraphsAnalyzed;
            if(numGraphsAnalyzed % 100 == 0){
                printf("Graphs of size %d analyzed: %d\n", GRAPH_SIZE, numGraphsAnalyzed);
            }
        }

        res.close();
        file.close();
    }

    // multithreaded version
    static void getGraphStatsFast(const std::string& path) {
        std::filesystem::path originalPath(path);
        std::filesystem::path newPath = originalPath.parent_path() / originalPath.stem();
        newPath += "_stats";
        newPath += originalPath.extension();

        // 1. Read all lines into memory sequentially
        std::ifstream file(path);
        if (!file.is_open()) {
            std::cerr << "Error: Could not open input file at " << path << std::endl;
            exit(1);
        }

        std::vector<std::string> lines;
        std::string line;
        while (std::getline(file, line)) {
            if (!line.empty()) {
                lines.push_back(line);
            }
        }
        file.close();

        size_t numTasks = lines.size();
        if (numTasks == 0) return;

        // Allocate space for results to preserve exact file order
        std::vector<std::vector<int>> results(numTasks);

        // 2. Set up multithreading controls
        std::atomic<size_t> nextTaskIndex{0};
        std::atomic<int> numGraphsAnalyzed{0};
        std::mutex coutMtx; // Prevents garbled console printing

        const std::vector<std::vector<int>> pinnacleSets = generatePinnacleSets();

        // Worker lambda function
        auto worker = [&]() {
            while (true) {
                // Safely grab the next line's index
                size_t taskIdx = nextTaskIndex.fetch_add(1);
                if (taskIdx >= numTasks) break; // No more lines to process

                const std::string& currentLine = lines[taskIdx];

                // Perform the heavy computation
                auto decoded_graph = decodeGraphG6(currentLine);
                Graph<GRAPH_SIZE> g(decoded_graph);
                results[taskIdx] = Graph::analyzeStats(g, pinnacleSets);

                // Safe progress tracking
                int processed = ++numGraphsAnalyzed;
                if (processed % 100 == 0) {
                    std::lock_guard<std::mutex> lock(coutMtx);
                    printf("Graphs of size %ld analyzed: %d\n", GRAPH_SIZE, processed);
                }
            }
        };

        // 3. Spawn workers based on system capabilities
        unsigned int numThreads = std::thread::hardware_concurrency();
        if (numThreads == 0) numThreads = 4; // Fallback

        std::vector<std::thread> threads;
        for (unsigned int i = 0; i < numThreads; ++i) {
            threads.emplace_back(worker);
        }

        // Wait for all threads to finish
        for (auto& t : threads) {
            t.join();
        }

        // 4. Write results back to the file sequentially
        std::ofstream res(newPath.string());
        if (!res.is_open()) {
            std::cerr << "Error: Could not create output file at " << newPath.string() << std::endl;
            exit(1);
        }

        for (size_t i = 0; i < numTasks; ++i) {
            res << lines[i] << ",[" << results[i][0] << "," << results[i][1] << "," << results[i][2] << "]\n";
        }

        if((int)numGraphsAnalyzed % 100 != 0){
            printf("Graphs of size %ld analyzed: %d\n", GRAPH_SIZE, (int)numGraphsAnalyzed);
        }

        res.close();
    }

    // decodes a graph6 formatted string into an adjacency matrix
    static std::vector<std::bitset<GRAPH_SIZE>> decodeGraphG6(const std::string& graph){
        if (graph.empty()) {
            return std::vector<std::bitset<GRAPH_SIZE>>(GRAPH_SIZE);
        }

        size_t idx = 0;
        size_t n = 0;

        // 1. Decode the number of vertices n
        if (graph[idx] == 126) {
            idx++;
            if (idx < graph.size() && graph[idx] == 126) {
                // 8-byte format: 126 126 followed by 6 bytes (36 bits)
                idx++;
                for (int i = 0; i < 6 && idx < graph.size(); ++i) {
                    n = (n << 6) | (graph[idx++] - 63);
                }
            } else {
                // 4-byte format: 126 followed by 3 bytes (18 bits)
                for (int i = 0; i < 3 && idx < graph.size(); ++i) {
                    n = (n << 6) | (graph[idx++] - 63);
                }
            }
        } else {
            // 1-byte format
            n = graph[idx++] - 63;
        }

        // Initialize adjacency matrix with the compile-time GRAPH_SIZE
        std::vector<std::bitset<GRAPH_SIZE>> res(GRAPH_SIZE);

        // Safety check to ensure we don't exceed the allocated bitset bounds
        size_t effective_n = std::min(n, GRAPH_SIZE);

        // 2. Decode the bit vector R(x)
        size_t bit_buffer = 0;
        int bits_available = 0;

        for (size_t col = 0; col < effective_n; ++col) {
            for (size_t row = 0; row < col; ++row) {
                // If we run out of bits in our current 6-bit chunk, fetch the next byte
                if (bits_available == 0) {
                    if (idx >= graph.size()) {
                        // Out of data prematurely
                        return res;
                    }
                    bit_buffer = graph[idx++] - 63;
                    bits_available = 6;
                }

                // Extract the most significant bit from the 6-bit block
                bool edge = (bit_buffer >> (bits_available - 1)) & 1;
                bits_available--;

                if (edge) {
                    res[row].set(col);
                    res[col].set(row); // Undirected graph symmetry
                }
            }
        }

        return res;
    }

    bool isConnected() const {
        // A graph with 0 or 1 vertices is vacuously connected
        if (GRAPH_SIZE <= 1) {
            return true;
        }

        // Keep track of visited vertices using a bitset
        std::bitset<GRAPH_SIZE> visited;
        std::queue<size_t> q;

        // Start BFS from vertex 0
        q.push(0);
        visited.set(0);
        size_t visited_count = 1;

        while (!q.empty()) {
            size_t curr = q.front();
            q.pop();

            // adjMatrix[curr] gives us a bitset of all neighbors.
            // We only care about neighbors that haven't been visited yet.
            std::bitset<GRAPH_SIZE> unvisited_neighbors = adjMatrix[curr] & ~visited;

            // If there are any unvisited neighbors, find them
            if (unvisited_neighbors.any()) {
                for (size_t next = 0; next < GRAPH_SIZE; ++next) {
                    if (unvisited_neighbors.test(next)) {
                        visited.set(next);
                        q.push(next);
                        visited_count++;

                        // Optimization: If we've visited all vertices, it's connected
                        if (visited_count == GRAPH_SIZE) {
                            return true;
                        }
                    }
                }
            }
        }

        // If BFS finished and we didn't visit every vertex, it's disconnected
        return visited_count == GRAPH_SIZE;
    }

    // removes each edge and counts { #decreased labelings, #same, #increased }
    static std::vector<int> analyzeStats(Graph &g, const std::vector<std::vector<int>> &basePinnacleSets){
        std::vector<int> res(3, 0);
        std::vector<std::bitset<GRAPH_SIZE>> adj = g.adjMatrix;

        for(size_t i = 0; i < GRAPH_SIZE; ++i){
            for(size_t j = i + 1; j < GRAPH_SIZE; ++j){
                if(adj[i][j] == 1){
                    adj[i][j] = 0;
                    adj[j][i] = 0;

                    Graph modifiedGraph(adj);

                    if(modifiedGraph.isConnected()){
                        std::vector<std::vector<int>> admissablePinSets;

                        // // get all admissable pin sets
                        for(const auto& p : basePinnacleSets){
                            std::vector<std::vector<int>> pinSets = g.getAdmissablePinnacleSets(p);
                            admissablePinSets.reserve(admissablePinSets.size() + pinSets.size());
                            admissablePinSets.insert(admissablePinSets.end(), std::make_move_iterator(pinSets.begin()), std::make_move_iterator(pinSets.end()));
                        }

                        // printf("Graph Size: %d\n", GRAPH_SIZE);
                        // printf("pinnacleSets.size(): %d\n", basePinnacleSets.size());
                        // printf("admissablePinSets.size(): %d\n\n", admissablePinSets.size());

                        for(const auto& p : admissablePinSets){
                            int validLabelingsBeforeBitFlip = 0;
                            int validLabelingsAfterBitFlip = 0;

                            g.resetValues();
                            g.countHeapPermutations(p, GRAPH_SIZE, validLabelingsBeforeBitFlip);

                            modifiedGraph.resetValues();
                            modifiedGraph.countHeapPermutations(p, GRAPH_SIZE, validLabelingsAfterBitFlip);

                            validLabelingsBeforeBitFlip += validLabelingsBeforeBitFlip;
                            validLabelingsAfterBitFlip += validLabelingsAfterBitFlip;

                            if(validLabelingsBeforeBitFlip > validLabelingsAfterBitFlip){
                                ++res[0];
                            } else if(validLabelingsBeforeBitFlip < validLabelingsAfterBitFlip){
                                ++res[2];
                            } else{ // originalGraphCount == newGraphCount
                                ++res[1];
                            }
                        }
                    }

                    adj[i][j] = 1;
                    adj[j][i] = 1;
                }
            }
        }

        return res;
    }

    static std::vector<std::vector<int>> generatePinnacleSets(){
        std::vector<std::vector<int>> basePinSets;

        // generate all valid pinnacle sets for g
        // start by making all base case pin sets of size 1 <= n < GRAPH_SIZE
        // ex { 2, 3, ..., k, GRAPH_SIZE }
        for(size_t k = 1; k < GRAPH_SIZE; ++k){
            std::vector<int> currPinSet(k);

            for(size_t l = 0; l < k - 1; ++l){
                currPinSet[l] = l + 2;
            }

            currPinSet[k - 1] = GRAPH_SIZE;
            basePinSets.push_back(currPinSet);
        }

        return basePinSets;
    }
};
