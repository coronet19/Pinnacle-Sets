#pragma once

#include <vector>
#include <cassert>
#include <algorithm>
#include <set>
#include <bit>
#include <iostream>
#include <fstream>
#include <filesystem>
#include <queue>
#include <thread>
#include <atomic>
#include <mutex>

#include "tui.hpp"


class Graph {
private:
    // Validates against a precomputed pinnacle bitmask — avoids rebuilding the mask
    // on every one of the n! leaf calls in countHeapPermutations.
    bool isValidLabelingFast(uint64_t pinnacleValueMask) const {
        for (size_t i = 0; i < graphSize; ++i) {
            int currentLabel = values[i];

            // Iterate only over existing edges
            uint64_t mask = adjMatrix[i];

            // A vertex is a pinnacle if it has neighbors AND
            // none of those neighbors have a higher label.
            bool hasNeighbors = (mask != 0);
            bool hasGreaterNeighbor = false;

            while (mask != 0) {
                // Get the index of the next set bit (neighbor)
                int j = std::countr_zero(mask);

                if (values[j] > currentLabel) {
                    hasGreaterNeighbor = true;
                    break;
                }

                // Clear the lowest set bit to move to the next neighbor
                mask &= mask - 1;
            }

            bool actsAsPinnacle = hasNeighbors && !hasGreaterNeighbor;
            bool shouldBePinnacle = (pinnacleValueMask >> (currentLabel - 1)) & 1;

            if (actsAsPinnacle != shouldBePinnacle) return false;
        }
        return true;
    }

    // Returns a bitmask with bit (label-1) set for every vertex that acts as a pinnacle under
    // the current labeling (has neighbors, none with a greater label). This is exactly the
    // per-vertex logic of isValidLabelingFast, but computed once instead of once per candidate
    // pinnacle set — a labeling is valid for set P iff this mask equals P's bitmask.
    uint64_t actualPinnacleValueMask() const {
        uint64_t result = 0;
        for (size_t i = 0; i < graphSize; ++i) {
            uint64_t mask = adjMatrix[i];
            if (mask == 0) continue; // no neighbors -> never a pinnacle (matches hasNeighbors)

            int currentLabel = values[i];
            bool hasGreaterNeighbor = false;
            while (mask != 0) {
                int j = std::countr_zero(mask);
                if (values[j] > currentLabel) { hasGreaterNeighbor = true; break; }
                mask &= mask - 1;
            }

            if (!hasGreaterNeighbor) result |= (1ULL << (currentLabel - 1));
        }
        return result;
    }

    // Inner Heap's algorithm — tallies the actual pinnacle value-mask of each of the n! labelings
    // into a histogram, so a single traversal yields counts for every pinnacle set at once.
    void computePinnacleHistogramInner(std::vector<uint64_t>& hist, int size) {
        if (size == 1) {
            ++hist[actualPinnacleValueMask()];
            return;
        }
        for (int i = 0; i < size; i++) {
            computePinnacleHistogramInner(hist, size - 1);
            if (size & 1) std::swap(values[0], values[size - 1]);
            else std::swap(values[i], values[size - 1]);
        }
    }

    // Inner Heap's algorithm — takes a precomputed mask so it isn't rebuilt at each leaf.
    void countHeapPermutationsInner(uint64_t pinnacleValueMask, int size, uint64_t& count) {
        if (size == 1) {
            if (isValidLabelingFast(pinnacleValueMask)) ++count;
            return;
        }
        for (int i = 0; i < size; i++) {
            countHeapPermutationsInner(pinnacleValueMask, size - 1, count);
            if (size & 1) std::swap(values[0], values[size - 1]);
            else std::swap(values[i], values[size - 1]);
        }
    }

    // Multi-pinnacle inner: precomputed masks avoid rebuilding per leaf, and all sets
    // are checked in one Heap's run instead of one run per set.
    void countHeapPermutationsInner(const std::vector<uint64_t>& masks, int size, uint64_t& count) {
        if (size == 1) {
            for (uint64_t m : masks)
                if (isValidLabelingFast(m)) ++count;
            return;
        }
        for (int i = 0; i < size; i++) {
            countHeapPermutationsInner(masks, size - 1, count);
            if (size & 1) std::swap(values[0], values[size - 1]);
            else std::swap(values[i], values[size - 1]);
        }
    }

    // Builds a bitmask where bit (label-1) is set for each label in the pinnacle set.
    static uint64_t buildPinnacleMask(const std::vector<int>& pinnacleSet) {
        uint64_t mask = 0;
        for (int val : pinnacleSet) mask |= (1ULL << (val - 1));
        return mask;
    }

public:
    const size_t graphSize;

    // Maps vertices to their edges, where an edge between two vertices exists if (adjMatrix[v] >> j) & 1 == 1.
    // i.e. adjMatrix[vertex] -> 0b00100101, where 0 <= vertex < graphSize,
    // and the 0-bits mean no edge, and the 1-bits mean an edge.
    const std::vector<uint64_t> adjMatrix;

    std::vector<int> values; // values[i] holds the value at vertex i

    inline Graph(size_t n, std::vector<uint64_t> graph)
        : graphSize(n), adjMatrix(std::move(graph)) {
        this->resetValues();
    }

    // Bitmask covering exactly the valid vertex range [0, graphSize)
    uint64_t fullMask() const {
        return (graphSize == 64) ? ~0ULL : (1ULL << graphSize) - 1;
    }

    // Gets the next lexicographic pinnacle set that maintains strictly increasing ordering.
    static bool getNextPinnacleSet(std::vector<int>& p, size_t graphSize) {
        if (p.size() < 2) return false;

        int k = static_cast<int>(p.size());
        int targetIdx = -1;

        for (int i = k - 2; i >= 0; --i) {
            int ceiling = static_cast<int>(graphSize) - (k - 1 - i);
            if (p[i] < ceiling) {
                targetIdx = i;
                break;
            }
        }

        if (targetIdx == -1) return false;

        p[targetIdx]++;
        for (int j = targetIdx + 1; j < k - 1; ++j)
            p[j] = p[j - 1] + 1;

        return true;
    }

    bool isValidLabeling(const std::vector<int>& pinnacleSet) const {
        if (pinnacleSet.empty() || pinnacleSet.back() != static_cast<int>(graphSize))
            return false;

        // Map pinnacle values for O(1) lookup; pinnacleSet contains 1-indexed labels
        return isValidLabelingFast(buildPinnacleMask(pinnacleSet));
    }

    // Takes in a base pinnacle set b of size n and returns
    // all pinnacle sets p of size n where p >= b
    std::vector<std::vector<int>> getAdmissablePinnacleSets(std::vector<int> pinnacles) {
        std::vector<std::vector<int>> admissable;
        std::sort(pinnacles.begin(), pinnacles.end());

        // A pinnacle set is admissible iff SOME labeling of this graph realizes it exactly.
        // Tally the actual pinnacle set of every one of the n! labelings into a histogram once;
        // then a candidate set is admissible iff its bucket is non-zero.
        //
        // The earlier approach tested a single "safe" assignment per independent set (sorted
        // pinnacle labels on the independent set, sorted non-pinnacle labels elsewhere). That is
        // a necessary but NOT sufficient condition: when that one arrangement failed, a set was
        // dropped even though another labeling realized it, silently undercounting admissible sets.
        std::vector<uint64_t> hist(1ULL << graphSize, 0);
        this->resetValues();
        this->computePinnacleHistogramInner(hist, (int)graphSize);

        do {
            if (pinnacles.empty() || pinnacles.back() != static_cast<int>(graphSize)) continue;

            if (hist[buildPinnacleMask(pinnacles)] > 0)
                admissable.push_back(pinnacles);

        } while (Graph::getNextPinnacleSet(pinnacles, graphSize));

        return admissable;
    }

    // Determines all independent sets of a given size n
    std::vector<std::vector<int>> getIndependentSets(int n) {
        std::vector<std::vector<int>> results;

        // P: candidate vertices (initially all), R: current independent set being built
        uint64_t initialP = fullMask();
        uint64_t initialR = 0;

        // Recursive backtracking
        auto findSets = [&](auto self, uint64_t P, uint64_t R) -> void {
            // Base case: reached target size
            if ((int)std::popcount(R) == n) {
                std::vector<int> setIndices;
                for (int i = 0; i < (int)graphSize; ++i)
                    if ((R >> i) & 1) setIndices.push_back(i);
                results.push_back(setIndices);
                return;
            }

            // Pruning: if remaining candidates + current set < target size, stop
            if ((int)std::popcount(R) + (int)std::popcount(P) < n) return;

            // Process candidates
            while (P != 0) {
                // Get the next candidate index
                int v = std::countr_zero(P);

                // 1. Remove v from the candidate pool P
                P &= P - 1;

                // 2. Create a new candidate pool: next candidates must NOT be adjacent to v
                uint64_t nextP = P & ~adjMatrix[v];

                // 3. Add v to the current result set R and recurse
                uint64_t nextR = R | (1ULL << v);

                self(self, nextP, nextR);
            }
        };

        findSets(findSets, initialP, initialR);
        return results;
    }

    void resetValues() {
        this->values.resize(graphSize);
        for (size_t i = 0; i < graphSize; ++i)
            this->values[i] = i + 1;
    }

    void setValues(const std::vector<int>& vals) {
        std::copy(vals.begin(), vals.end(), this->values.begin());
    }

    // Counts labelings matching any pinnacle set in the list; precomputes masks up front
    // so they aren't rebuilt at each of the n! leaf calls.
    void countHeapPermutations(const std::vector<std::vector<int>>& pinnacles, int size, uint64_t& count) {
        std::vector<uint64_t> masks;
        masks.reserve(pinnacles.size());
        for (const auto& p : pinnacles) masks.push_back(buildPinnacleMask(p));
        countHeapPermutationsInner(masks, size, count);
    }

    // Counts labelings matching a single pinnacle set; precomputes the mask once
    // so it isn't rebuilt at each of the n! leaf calls.
    void countHeapPermutations(const std::vector<int>& p, int size, uint64_t& count) {
        countHeapPermutationsInner(buildPinnacleMask(p), size, count);
    }

    // Counts valid labelings for every given pinnacle set in a SINGLE n! traversal.
    // Returns counts[k] = number of labelings whose pinnacle set is exactly pinnacleSets[k].
    // `hist` is caller-owned scratch of size (1 << graphSize), reused across calls to avoid
    // reallocating per edge. Each leaf's actual pinnacle set is bucketed once; per-set counts
    // are then O(1) lookups, replacing one full n! walk per pinnacle set.
    std::vector<uint64_t> countLabelingsForAll(const std::vector<std::vector<int>>& pinnacleSets,
                                               std::vector<uint64_t>& hist) {
        std::fill(hist.begin(), hist.end(), 0);
        resetValues();
        computePinnacleHistogramInner(hist, (int)graphSize);

        std::vector<uint64_t> counts(pinnacleSets.size());
        for (size_t k = 0; k < pinnacleSets.size(); ++k)
            counts[k] = hist[buildPinnacleMask(pinnacleSets[k])];
        return counts;
    }

    void printGraph() const {
        for (size_t i = 0; i < adjMatrix.size(); ++i) {
            for (size_t j = 0; j < graphSize; ++j)
                std::cout << ((adjMatrix[i] >> j) & 1);
            std::cout << std::endl;
        }
    }

    static void printGraph(const std::string& graph) {
        if (graph.empty()) return;

        size_t idx = 0;
        size_t graphSize = 0;

        // 1. Decode the number of vertices n
        if (graph[idx] == 126) {
            idx++;
            if (idx < graph.size() && graph[idx] == 126) {
                idx++;
                for (int i = 0; i < 6 && idx < graph.size(); ++i)
                    graphSize = (graphSize << 6) | (graph[idx++] - 63);
            } else {
                for (int i = 0; i < 3 && idx < graph.size(); ++i)
                    graphSize = (graphSize << 6) | (graph[idx++] - 63);
            }
        } else {
            graphSize = graph[idx++] - 63;
        }

        std::vector<unsigned long> res(graphSize, 0);

        // 2. Decode the bit vector R(x)
        size_t bit_buffer = 0;
        int bits_available = 0;

        for (size_t col = 0; col < graphSize; ++col) {
            for (size_t row = 0; row < col; ++row) {
                if (bits_available == 0) {
                    if (idx >= graph.size()) break;
                    bit_buffer = graph[idx++] - 63;
                    bits_available = 6;
                }
                bool edge = (bit_buffer >> (bits_available - 1)) & 1;
                bits_available--;
                if (edge) {
                    res[row] |= (1UL << (graphSize - col - 1));
                    res[col] |= (1UL << (graphSize - row - 1));
                }
            }
        }

        // 3. Print the adjacency matrix
        for (size_t i = 0; i < graphSize; ++i) {
            for (size_t j = 0; j < graphSize; ++j)
                std::cout << (bool)(res[i] & (1UL << (graphSize - j - 1)));
            std::cout << std::endl;
        }
    }

    static std::vector<uint64_t> makeCompleteGraph(size_t graphSize, const std::vector<int>& pinnacleSet) {
        std::vector<uint64_t> res(graphSize, 0);

        // Pinnacle lookup via bitmask (O(1)); pinnacleSet contains 1-indexed vertex values
        uint64_t isPinnacle = 0;
        for (int p : pinnacleSet)
            if (p > 0 && p <= (int)graphSize)
                isPinnacle |= (1ULL << p);

        uint64_t completeMask = (graphSize == 64) ? ~0ULL : (1ULL << graphSize) - 1;

        for (size_t i = 0; i < graphSize; ++i) {
            int vertexValue = (int)i + 1;
            if ((isPinnacle >> vertexValue) & 1) {
                // Pinnacle: connects only to vertices with smaller indices
                res[i] = (1ULL << i) - 1;
            } else {
                // Non-pinnacle: connects to all except self
                res[i] = completeMask ^ (1ULL << i);
            }
        }

        return res;
    }

    // Decodes a graph6 formatted string into an adjacency matrix
    static std::vector<uint64_t> decodeGraphG6(size_t graphSize, const std::string& graph) {
        if (graph.empty()) return std::vector<uint64_t>(graphSize, 0);

        size_t idx = 0;
        size_t n = 0;

        // 1. Decode the number of vertices n
        if (graph[idx] == 126) {
            idx++;
            if (idx < graph.size() && graph[idx] == 126) {
                // 8-byte format: 126 126 followed by 6 bytes (36 bits)
                idx++;
                for (int i = 0; i < 6 && idx < graph.size(); ++i)
                    n = (n << 6) | (graph[idx++] - 63);
            } else {
                // 4-byte format: 126 followed by 3 bytes (18 bits)
                for (int i = 0; i < 3 && idx < graph.size(); ++i)
                    n = (n << 6) | (graph[idx++] - 63);
            }
        } else {
            // 1-byte format
            n = graph[idx++] - 63;
        }

        // Safety check: don't exceed the allocated adjacency matrix bounds
        std::vector<uint64_t> res(graphSize, 0);
        size_t effective_n = std::min(n, graphSize);

        // 2. Decode the bit vector R(x)
        size_t bit_buffer = 0;
        int bits_available = 0;

        for (size_t col = 0; col < effective_n; ++col) {
            for (size_t row = 0; row < col; ++row) {
                // If we run out of bits in the current 6-bit chunk, fetch the next byte
                if (bits_available == 0) {
                    if (idx >= graph.size()) return res;
                    bit_buffer = graph[idx++] - 63;
                    bits_available = 6;
                }

                // Extract the most significant bit from the 6-bit block
                bool edge = (bit_buffer >> (bits_available - 1)) & 1;
                bits_available--;

                if (edge) {
                    res[row] |= (1ULL << col);
                    res[col] |= (1ULL << row); // undirected graph symmetry
                }
            }
        }

        return res;
    }

    bool isConnected() const {
        // A graph with 0 or 1 vertices is vacuously connected
        if (graphSize <= 1) return true;

        // BFS from vertex 0
        uint64_t visited = 1ULL;
        std::queue<size_t> q;
        q.push(0);
        size_t visited_count = 1;

        while (!q.empty()) {
            size_t curr = q.front();
            q.pop();

            // Only consider neighbors not yet visited
            uint64_t unvisited_neighbors = adjMatrix[curr] & ~visited;

            while (unvisited_neighbors != 0) {
                int next = std::countr_zero(unvisited_neighbors);
                unvisited_neighbors &= unvisited_neighbors - 1;
                visited |= (1ULL << next);
                q.push(next);

                // Early exit once all vertices have been reached
                if (++visited_count == graphSize) return true;
            }
        }

        // If BFS finished without visiting all vertices, the graph is disconnected
        return visited_count == graphSize;
    }

    // Per-pinnacle-set breakdown of the edge-removal stats, optionally produced by
    // analyzeStats. For one admissible pinnacle set: how many edge removals decreased,
    // kept the same, or increased the number of valid labelings.
    struct PinSetStat {
        std::vector<int> pinnacleSet;
        int decreased = 0;
        int same = 0;
        int increased = 0;
    };

    // Removes each edge and counts { #decreased labelings, #same, #increased }.
    // Each n! traversal is shared across all admissible pinnacle sets via a histogram of
    // actual pinnacle sets (see countLabelingsForAll), instead of one walk per set.
    //
    // If perSet is non-null it is filled with one PinSetStat per admissible pinnacle set,
    // giving the same decreased/same/increased breakdown but split out by pinnacle set.
    static std::vector<int> analyzeStats(Graph& g, const std::vector<std::vector<int>>& basePinnacleSets,
                                         std::vector<PinSetStat>* perSet = nullptr) {
        std::vector<int> res(3, 0);
        std::vector<uint64_t> adj = g.adjMatrix;

        // Scratch histogram indexed by pinnacle value-mask, reused across the "before" count
        // and every edge removal. Size 1 << graphSize covers every possible mask.
        std::vector<uint64_t> hist(1ULL << g.graphSize, 0);

        // Histogram of g's pinnacle sets across all n! labelings. A candidate set is admissible
        // iff its bucket is non-zero, and that bucket IS its "before" count — so a single n!
        // walk yields both the admissible list and the before counts (no separate per-set work,
        // and no per-edge recompute of admissibility, which depends only on g).
        g.resetValues();
        g.computePinnacleHistogramInner(hist, (int)g.graphSize);

        // The admissible pinnacle sets depend only on g, not on which edge is being removed.
        // Enumerate the candidate sets (each base set and every set >= it of the same size,
        // which together cover all pinnacle sets containing graphSize) and keep the realized ones.
        std::vector<std::vector<int>> admissablePinSets;
        std::vector<uint64_t> labelingsBefore;
        for (const auto& base : basePinnacleSets) {
            std::vector<int> p = base;
            std::sort(p.begin(), p.end());
            do {
                if (p.empty() || p.back() != static_cast<int>(g.graphSize)) continue;
                uint64_t before = hist[buildPinnacleMask(p)];
                if (before > 0) {
                    admissablePinSets.push_back(p);
                    labelingsBefore.push_back(before);
                }
            } while (Graph::getNextPinnacleSet(p, g.graphSize));
        }

        if (perSet) {
            perSet->clear();
            perSet->reserve(admissablePinSets.size());
            for (const auto& ps : admissablePinSets)
                perSet->push_back({ps, 0, 0, 0});
        }

        for (size_t i = 0; i < g.graphSize; ++i) {
            for (size_t j = i + 1; j < g.graphSize; ++j) {
                if ((adj[i] >> j) & 1) {
                    adj[i] &= ~(1ULL << j);
                    adj[j] &= ~(1ULL << i);

                    Graph modifiedGraph(g.graphSize, adj);

                    if (modifiedGraph.isConnected()) {
                        std::vector<uint64_t> after = modifiedGraph.countLabelingsForAll(admissablePinSets, hist);
                        for (size_t k = 0; k < admissablePinSets.size(); ++k) {
                            if (labelingsBefore[k] > after[k]) {
                                ++res[0];
                                if (perSet) ++(*perSet)[k].decreased;
                            } else if (labelingsBefore[k] < after[k]) {
                                ++res[2];
                                if (perSet) ++(*perSet)[k].increased;
                            } else {
                                ++res[1];
                                if (perSet) ++(*perSet)[k].same;
                            }
                        }
                    }

                    adj[i] |= (1ULL << j);
                    adj[j] |= (1ULL << i);
                }
            }
        }

        return res;
    }

    // Reference implementation kept as a correctness/benchmark oracle for analyzeStats.
    // Walks all n! permutations once per admissible pinnacle set (the pre-histogram approach).
    static std::vector<int> analyzeStatsLegacy(Graph& g, const std::vector<std::vector<int>>& basePinnacleSets) {
        std::vector<int> res(3, 0);
        std::vector<uint64_t> adj = g.adjMatrix;

        std::vector<std::vector<int>> admissablePinSets;
        for (const auto& p : basePinnacleSets) {
            auto pinSets = g.getAdmissablePinnacleSets(p);
            admissablePinSets.insert(admissablePinSets.end(),
                std::make_move_iterator(pinSets.begin()),
                std::make_move_iterator(pinSets.end()));
        }

        std::vector<uint64_t> labelingsBefore(admissablePinSets.size(), 0);
        for (size_t k = 0; k < admissablePinSets.size(); ++k) {
            g.resetValues();
            g.countHeapPermutations(admissablePinSets[k], g.graphSize, labelingsBefore[k]);
        }

        for (size_t i = 0; i < g.graphSize; ++i) {
            for (size_t j = i + 1; j < g.graphSize; ++j) {
                if ((adj[i] >> j) & 1) {
                    adj[i] &= ~(1ULL << j);
                    adj[j] &= ~(1ULL << i);

                    Graph modifiedGraph(g.graphSize, adj);

                    if (modifiedGraph.isConnected()) {
                        for (size_t k = 0; k < admissablePinSets.size(); ++k) {
                            uint64_t after = 0;
                            modifiedGraph.resetValues();
                            modifiedGraph.countHeapPermutations(admissablePinSets[k], g.graphSize, after);

                            if (labelingsBefore[k] > after) ++res[0];
                            else if (labelingsBefore[k] < after) ++res[2];
                            else ++res[1];
                        }
                    }

                    adj[i] |= (1ULL << j);
                    adj[j] |= (1ULL << i);
                }
            }
        }

        return res;
    }

    // Generates the base pinnacle set of each size for a graph of size graphSize:
    // { {graphSize}, {2, graphSize}, {2, 3, graphSize}, ... }
    static std::vector<std::vector<int>> generatePinnacleSets(size_t graphSize) {
        std::vector<std::vector<int>> basePinSets;

        for (size_t k = 1; k < graphSize; ++k) {
            std::vector<int> currPinSet(k);
            for (size_t l = 0; l < k - 1; ++l)
                currPinSet[l] = l + 2;
            currPinSet[k - 1] = graphSize;
            basePinSets.push_back(currPinSet);
        }

        return basePinSets;
    }

    struct TaskResult {
        size_t index;
        std::string line;
        std::vector<int> stats;
        std::vector<PinSetStat> perSet; // populated only when extra logging is enabled
        bool operator>(const TaskResult& other) const { return index > other.index; }
    };

    static void getGraphStats(size_t graphSize, const std::string& path) {
        std::filesystem::path originalPath(path);
        std::filesystem::path newPath = originalPath.parent_path() / originalPath.stem();
        newPath += "_stats";
        newPath += originalPath.extension();

        std::ifstream file(path);
        if (!file.is_open()) {
            std::cerr << "Error: Could not open input file at " << path << std::endl;
            exit(1);
        }

        std::ofstream res(newPath.string());
        if (!res.is_open()) {
            std::cerr << "Error: Could not create output file at " << newPath.string() << std::endl;
            exit(1);
        }

        const std::vector<std::vector<int>> pinnacleSets = generatePinnacleSets(graphSize);
        int numGraphsAnalyzed = 0;
        std::string line;
        while (std::getline(file, line)) {
            if (line.empty()) continue;

            auto decoded_graph = decodeGraphG6(graphSize, line);
            Graph g(graphSize, decoded_graph);
            std::vector<int> stats = Graph::analyzeStats(g, pinnacleSets);

            res << line << ",[" << stats[0] << "," << stats[1] << "," << stats[2] << "]\n";

            ++numGraphsAnalyzed;
            if (numGraphsAnalyzed % 100 == 0)
                printf("Graphs of size %zu analyzed: %d\n", graphSize, numGraphsAnalyzed);
        }

        res.close();
        file.close();
    }

    static void getGraphStatsFast(size_t graphSize, const std::string& path,
                                  ProgressState* progress = nullptr, bool extra = false) {
        // std::filesystem::path originalPath(path);
        // std::filesystem::path newPath = originalPath.parent_path() / originalPath.stem();
        // newPath += "_stats.csv";

        // // When extra logging is enabled, per-pinnacle-set rows go to a sibling file:
        // //   <graph6>,<pinnacle set>,<#decreased>,<#same>,<#increased>
        // std::filesystem::path extraPath = originalPath.parent_path() / originalPath.stem();
        // extraPath += "_stats_extra.csv";
        std::filesystem::path originalPath(path);

        std::filesystem::path statsDir = originalPath.parent_path().parent_path() / "stats";

        std::filesystem::path newPath = statsDir / originalPath.stem();
        newPath += "_stats.csv";

        std::filesystem::path extraPath = statsDir / originalPath.stem();
        extraPath += "_stats_extra.csv";

        // 1. Read all lines into memory
        std::ifstream file(path);
        if (!file.is_open()) {
            std::cerr << "Error: Could not open input file at " << path << std::endl;
            exit(1);
        }

        std::vector<std::string> lines;
        std::string line;
        while (std::getline(file, line))
            if (!line.empty()) lines.push_back(line);
        file.close();

        size_t numTasks = lines.size();
        if (numTasks == 0) return;

        // Check if output file exists and count already-processed lines (resume support)
        size_t existingCount = 0;
        if (std::filesystem::exists(newPath)) {
            std::ifstream checkFile(newPath);
            std::string dummy;
            while (std::getline(checkFile, dummy))
                if (!dummy.empty()) existingCount++;
            checkFile.close();

            if (existingCount >= numTasks) {
                std::cout << "All graphs have already been processed." << std::endl;
                return;
            }
            std::cout << "Resuming from graph index: " << existingCount << std::endl;
        }

        // Open output file in append mode immediately
        std::ofstream res(newPath.string(), std::ios::app);
        if (!res.is_open()) {
            std::cerr << "Error: Could not open output file at " << newPath.string() << std::endl;
            exit(1);
        }

        // Per-pinnacle-set log. Appended in lockstep with the main stats file, so resume
        // stays consistent as long as --extra is used consistently (or with --force).
        std::ofstream extraRes;
        if (extra) {
            extraRes.open(extraPath.string(), std::ios::app);
            if (!extraRes.is_open()) {
                std::cerr << "Error: Could not open output file at " << extraPath.string() << std::endl;
                exit(1);
            }
        }

        // 2. Multithreading & synchronization controls
        std::atomic<size_t> nextTaskIndex{existingCount};
        std::atomic<int> numGraphsAnalyzed{(int)existingCount};

        // We expect the next write to be the first unprocessed index
        size_t nextWriteIndex = existingCount;

        std::mutex writeMtx; // protects the priority queue, file stream, and nextWriteIndex
        std::priority_queue<TaskResult, std::vector<TaskResult>, std::greater<TaskResult>> pq;

        const std::vector<std::vector<int>> pinnacleSets = generatePinnacleSets(graphSize);

        // Initialize the live-progress state (if a TUI is attached) now that the
        // resume offset and task count are known.
        if (progress) {
            progress->graphSize.store(graphSize);
            progress->total.store(numTasks);
            progress->startCount.store(existingCount);
            progress->processed.store(existingCount);
            progress->startTime = std::chrono::steady_clock::now();
        }

        // Worker lambda — computation is done in parallel, writes are serialized via the priority queue
        auto worker = [&]() {
            while (true) {
                size_t taskIdx = nextTaskIndex.fetch_add(1);
                if (taskIdx >= numTasks) break;

                const std::string& currentLine = lines[taskIdx];

                // Heavy computation (no locks held here)
                auto decoded_graph = decodeGraphG6(graphSize, currentLine);
                Graph g(graphSize, decoded_graph);
                if (progress) progress->setGraph(currentLine, g.adjMatrix, graphSize);
                std::vector<PinSetStat> perSet;
                auto stats = Graph::analyzeStats(g, pinnacleSets, extra ? &perSet : nullptr);

                int processed = ++numGraphsAnalyzed;
                if (progress) progress->processed.store((size_t)processed);

                // Critical section: push result and flush sequential entries to disk
                {
                    std::lock_guard<std::mutex> lock(writeMtx);

                    pq.push({taskIdx, currentLine, std::move(stats), std::move(perSet)});

                    // Drain the queue as long as the lowest index matches the expected sequence
                    while (!pq.empty() && pq.top().index == nextWriteIndex) {
                        TaskResult readyTask = pq.top();
                        pq.pop();
                        res << readyTask.line << "," << readyTask.stats[0] << ","
                            << readyTask.stats[1] << "," << readyTask.stats[2] << "\n";

                        if (extra) {
                            for (const auto& ps : readyTask.perSet) {
                                extraRes << readyTask.line << ",[";
                                for (size_t m = 0; m < ps.pinnacleSet.size(); ++m) {
                                    if (m) extraRes << ' ';
                                    extraRes << ps.pinnacleSet[m];
                                }
                                extraRes << "]," << ps.decreased << "," << ps.same
                                         << "," << ps.increased << "\n";
                            }
                        }

                        nextWriteIndex++;
                    }

                    // Force OS flush so progress is safe if the process is killed
                    res.flush();
                    if (extra) extraRes.flush();

                    if (!progress && processed % 100 == 0)
                        printf("Graphs of size %zu analyzed: %d\n", graphSize, processed);
                }
            }
        };

        // 3. Spawn workers
        unsigned int numThreads = std::thread::hardware_concurrency();
        if (numThreads == 0) numThreads = 4;

        std::vector<std::thread> threads;
        for (unsigned int i = 0; i < numThreads; ++i)
            threads.emplace_back(worker);

        for (auto& t : threads)
            t.join();

        if (!progress && (int)numGraphsAnalyzed % 100 != 0)
            printf("Graphs of size %zu analyzed: %d\n", graphSize, (int)numGraphsAnalyzed);

        res.close();
        if (extra) extraRes.close();
    }
};
