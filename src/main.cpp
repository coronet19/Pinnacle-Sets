#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <algorithm>

#include "../include./permutations.h"


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

    std::vector<std::vector<uint8_t>> pinnacleSets = Permutations::createPinnacleSets(m, n);
    std::vector<std::vector<uint8_t>> perms = Permutations::getPermutations(m + n);

    for(const auto& pinnacleSet : pinnacleSets){
        int count = 0;

        for(const auto& perm : perms){
            count += Permutations::isValidLabeling(perm, pinnacleSet, m, n);
        }

        if(count == 0) continue;

        std::cout << "Pinnacle Set: {";
        for(int i = 0; i < pinnacleSet.size(); ++i){
            std::cout << (int)pinnacleSet[i] << ((i == pinnacleSet.size() - 1) ? "}, " : ", ");
        }

        std::cout << "Valid Labelings: " << count << std::endl;
    }

    return 0;
}
