#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <algorithm>

#include "../include/permutations.h"

long long factorial(long long n){
    if(n == 1 || n == 0) return 1;

    if(n < 0){
        std::cout << "n<0 in factorial" << std::endl;
        return 0;
    }

    return n * factorial(n - 1);
}

int nPr(int n, int r){
    return factorial(n) / factorial(n - r);
}

int nCr(int n, int r){
    return factorial(n) / (factorial(r) * factorial(n - r));
}

int main(int argc, char** argv){
    int m, n;
    std::string nums;

    // Assumes smallest labeling of the set is '1'.
    // m+n is always an element in the pinnacle set.
    // std::vector<int> pinnacleSet;

    std::cout << "Enter m: ";
    std::cin >> m;

    std::cout << "Enter n: ";
    std::cin >> n;

    // buffer formatting or something
    // std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    // std::cout << "Enter Pinnacle Set: ";
    // std::getline(std::cin, nums);
    // parse pinnacle set into vector<int>
    // std::stringstream ss(nums);
    // for(int temp; ss >> temp;){
    //     pinnacleSet.push_back(temp);
    // }
    // sort(pinnacleSet.begin(), pinnacleSet.end());

    // std::vector<std::vector<uint8_t>> pinnacleSets = Permutations::createPinnacleSets(m, n);
    std::vector<std::vector<uint8_t>> pinnacleSets = Permutations::createBipartitePinnacleSets(m, n);
    std::vector<std::vector<uint8_t>> perms = Permutations::getPermutations(m + n);
    int total = 0;

    for(const auto& pinnacleSet : pinnacleSets){
        int count = 0;

        for(const auto& perm : perms){
            count += Permutations::isValidLabeling(perm, pinnacleSet, m, n);
        }

        if(count == 0) continue;
        total += count;

        std::cout << "Pinnacle Set: {";
        for(int i = 0; i < pinnacleSet.size(); ++i){
            std::cout << (int)pinnacleSet[i] << ((i == pinnacleSet.size() - 1) ? "}, " : ", ");
        }

        double r = (double)count / ((double)factorial(m) * (double)factorial(n));
        std::cout << "Valid Labelings: "
            << count
            << " = "
            << m
            << "! * "
            << n
            << "! * "
            << r
            << std::endl;

        int a = std::max(m, n) - pinnacleSet.size() + 1;
        int res = -1;

        // if(pinnacleSet.size() > (std::max(m, n) - std::min(m, n) + 1)){
        //     res = (a * (a + 1) * factorial(m) * factorial(n)) / 2;
        //     std::cout << "res: " << res << std::endl;
        // } else{
        //     // res = factorial(m + n) / std::pow(2, pinnacleSet.size());
        // }

        // if(pinnacleSet.size() > std::min(m, n)){
            // res = nPr(std::max(m, n), pinnacleSet.size());
            // res = factorial(pinnacleSet.size()) * nPr(std::max(m, n), pinnacleSet.size());
            // std::cout << "res: " << res << std::endl;
        // }
        // res = factorial(pinnacleSet.size()) * nPr(std::max(m, n), pinnacleSet.size());
    }

    std::cout << "Sum = "
        << total
        << " = "
        << m + n
        << "! = "
        << m
        << "! * "
        << n
        << "! * "
        << nCr(m + n, std::max(m, n))
        << std::endl;


    return 0;
}
