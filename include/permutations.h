#pragma once

#include <vector>
#include <cstdint> // defines uint8_t
#include <cassert>
#include <algorithm>
#include <ranges>
#include <random>
#include <map>
#include <set>


class Permutations{
private:
    static size_t getRandomIndex(const size_t s);
    static void backtrack(int start, int n, std::vector<uint8_t>& current, std::vector<std::vector<uint8_t>>& res);
    static std::vector<uint8_t> reduce(const std::vector<uint8_t>& s1, const std::vector<uint8_t>& s2);
    static int getOverlap(const std::vector<uint8_t>& s1, const std::vector<uint8_t>& s2);

public:
    static std::vector<std::vector<uint8_t>> getPermutations(int n);
    static std::vector<uint8_t> getSuperpermutation(std::vector<std::vector<uint8_t>> perms);
    static std::vector<uint8_t> getSuperpermutationFast(std::vector<std::vector<uint8_t>> perms);
    static bool isSuperpermutation(const std::vector<std::vector<uint8_t>>& perms, const std::vector<uint8_t>& superpermutation);
    static std::vector<std::vector<uint8_t>> createOverlapMatrix(const std::vector<std::vector<uint8_t>>& perms);
    static std::vector<std::vector<uint8_t>> createPinnacleSets(int m, int n);
    static bool isValidLabeling(std::vector<uint8_t> perm, std::vector<uint8_t> pinnacleSet, int m, int n);
};
