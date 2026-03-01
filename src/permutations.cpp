#include "../include/permutations.h"


size_t Permutations::getRandomIndex(const size_t s){
    static std::random_device rd;
    static std::mt19937 gen(rd()); // initialized only once
    std::uniform_int_distribution<size_t> dist(0, s - 1);
    return dist(gen);
}

void Permutations::backtrack(int start, int n, std::vector<uint8_t>& current, std::vector<std::vector<uint8_t>>& res){
    if(start == n){
        res.push_back(current);
        return;
    }

    for(int i = start; i < n; ++i){
        std::swap(current[start], current[i]);
        backtrack(start + 1, n, current, res);
        std::swap(current[start], current[i]); // backtrack
    }
}

// returns the shortest order preserving vector containing s1 and s2
std::vector<uint8_t> Permutations::reduce(const std::vector<uint8_t>& s1, const std::vector<uint8_t>& s2){
    // checks if one sequence contains another
    if (std::search(s1.begin(), s1.end(), s2.begin(), s2.end()) != s1.end()) return s1;
    if (std::search(s2.begin(), s2.end(), s1.begin(), s1.end()) != s2.end()) return s2;

    int overlap12 = getOverlap(s1, s2);
    int overlap21 = getOverlap(s2, s1);

    if(overlap12 >= overlap21){
        std::vector<uint8_t> res = s1;
        res.insert(res.end(), s2.begin() + overlap12, s2.end());
        return res;
    } else{
        std::vector<uint8_t> res = s2;
        res.insert(res.end(), s1.begin() + overlap21, s1.end());
        return res;
    }
}

int Permutations::getOverlap(const std::vector<uint8_t>& s1, const std::vector<uint8_t>& s2){
    int maxPossible = std::min(s1.size(), s2.size());

    for(int len = maxPossible; len > 0; --len){
        if(std::equal(s1.end() - len, s1.end(), s2.begin())){
            return len;
        }
    }

    return 0;
}

std::vector<std::vector<uint8_t>> Permutations::getPermutations(int n) {
    std::vector<std::vector<uint8_t>> res;

    int total = 1;
    for (int i = 2; i <= n; ++i) total *= i;
    res.reserve(total);

    std::vector<uint8_t> current(n);
    for (int i = 0; i < n; ++i) current[i] = i + 1;

    backtrack(0, n, current, res);

    return res;
}

// uses a greedy approach to make a minimum sized superpermutation
std::vector<uint8_t> Permutations::getSuperpermutation(std::vector<std::vector<uint8_t>> perms){
    if (perms.empty()) return {};

    // size_t r = getRandomIndex(perms.size());
    size_t r = 0;

    std::vector<uint8_t> result = perms[r];
    std::vector<bool> used(perms.size(), false);
    used[r] = true;

    for(size_t count = 1; count < perms.size(); ++count){
        // (overlap amount, index)
        std::vector<std::vector<int>> bestOverlaps = { { -1, -1 } };

        for(size_t j = 0; j < perms.size(); ++j){
            if(used[j]) continue;

            // Use the overlap logic we built earlier
            int overlap = getOverlap(result, perms[j]);
            if(overlap > bestOverlaps[0][0]){
                bestOverlaps.clear();
                bestOverlaps = { { overlap, static_cast<int>(j) } };
            } else if(overlap == bestOverlaps[0][0]){
                bestOverlaps.push_back(std::vector<int>{ overlap, static_cast<int>(j) });
            }
        }

        // size_t rnd = getRandomIndex(bestOverlaps.size());
        size_t rnd = 0;
        int bestOverlap = bestOverlaps[rnd][0];
        int bestIdx = bestOverlaps[rnd][1];

        int sum = 0;
        for(const auto& v : bestOverlaps){
            sum += v[0];
        }

        assert((int)(bestOverlaps[0][0]) == (int)((int)sum / (int)bestOverlaps.size()));

        // Append the unique part of the best match
        result.insert(result.end(), perms[bestIdx].begin() + bestOverlap, perms[bestIdx].end());
        used[bestIdx] = true;
    }

    return result;
}

// warning: vibecoded slop
// uses rb tree to store elements, and only calculates changed groups of permutations
std::vector<uint8_t> Permutations::getSuperpermutationFast(std::vector<std::vector<uint8_t>> perms) {
    if (perms.empty()) return {};

    // Map acts as our Red-Black Tree.
    // Key: Prefix of a permutation. Value: Set of indices starting with that prefix.
    std::map<std::vector<uint8_t>, std::set<size_t>> prefixMap;
    std::vector<bool> used(perms.size(), false);
    const size_t permSize = perms[0].size();

    // 1. Build the initial tree: Only done once.
    for (size_t i = 0; i < perms.size(); ++i) {
        for (size_t len = 1; len <= permSize; ++len) {
            std::vector<uint8_t> prefix(perms[i].begin(), perms[i].begin() + len);
            prefixMap[prefix].insert(i);
        }
    }

    size_t currentIdx = 0;
    std::vector<uint8_t> result = perms[currentIdx];
    used[currentIdx] = true;

    // Helper to purge used elements from the tree (keeping groups updated)
    auto markAsUsed = [&](size_t idx) {
        used[idx] = true;
        for (size_t len = 1; len <= permSize; ++len) {
            std::vector<uint8_t> prefix(perms[idx].begin(), perms[idx].begin() + len);
            prefixMap[prefix].erase(idx);
        }
    };

    markAsUsed(currentIdx);

    for (size_t count = 1; count < perms.size(); ++count) {
        bool found = false;

        // 2. Only check relevant groups by looking up the current suffix in the RB-Tree
        for (int overlapLen = permSize - 1; overlapLen >= 1; --overlapLen) {
            std::vector<uint8_t> suffix(result.end() - overlapLen, result.end());

            auto it = prefixMap.find(suffix);
            if (it != prefixMap.end() && !it->second.empty()) {
                // Grab the first available permutation in this specific prefix group
                size_t nextIdx = *it->second.begin();

                // Append only the non-overlapping part
                result.insert(result.end(), perms[nextIdx].begin() + overlapLen, perms[nextIdx].end());

                currentIdx = nextIdx;
                markAsUsed(currentIdx);
                found = true;
                break;
            }
        }

        // 3. Fallback: If no overlap group is found, pick the first available permutation
        if (!found) {
            for (size_t i = 0; i < perms.size(); ++i) {
                if (!used[i]) {
                    result.insert(result.end(), perms[i].begin(), perms[i].end());
                    currentIdx = i;
                    markAsUsed(currentIdx);
                    break;
                }
            }
        }
    }

    return result;
}


bool Permutations::isSuperpermutation(const std::vector<std::vector<uint8_t>>& perms, const std::vector<uint8_t>& superpermutation){
    for(const auto& p : perms){
        auto it = std::search(superpermutation.begin(), superpermutation.end(), p.begin(), p.end());

        if(it == superpermutation.end()){
            return false;
        }
    }

    return true;
}


std::vector<std::vector<uint8_t>> Permutations::createOverlapMatrix(const std::vector<std::vector<uint8_t>>& perms){
    std::vector<std::vector<uint8_t>> res(perms.size(), std::vector<uint8_t>(perms.size()));

    return res;
}
