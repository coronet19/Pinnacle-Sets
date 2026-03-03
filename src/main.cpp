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
    // std::vector<std::vector<uint8_t>> perms = Permutations::getPermutations(m + n);
    std::vector<int> counts(pinnacleSets.size(), 0);
    int total = 0;

    std::vector<uint8_t> perm;
    for(int i = 1; i <= m + n; ++i){
        perm.push_back(i);
    }

    do{
        for(int i = 0; i < pinnacleSets.size(); ++i){
            counts[i] += Permutations::isValidLabeling(perm, pinnacleSets[i], m, n);
        }
    } while(std::next_permutation(perm.begin(), perm.end()));

    for(int i = 0; i < counts.size(); ++i){
        int count = counts[i];
        const auto& pinnacleSet = pinnacleSets[i];

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
