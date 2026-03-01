#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <algorithm>

#include "../include./permutations.h"


int main(int argc, char** argv){
    int m, n;
    std::string nums;
    std::vector<int> pinnacleSet;

    std::cout << "Enter m: ";
    std::cin >> m;

    std::cout << "Enter n: ";
    std::cin >> n;

    // buffer formatting or something
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

    std::cout << "Enter Pinnacle Set: ";
    std::getline(std::cin, nums);

    // parse pinnacle set into vector<int>
    std::stringstream ss(nums);
    for(int temp; ss >> temp;){
        pinnacleSet.push_back(temp);
    }

    sort(pinnacleSet.begin(), pinnacleSet.end());
    std::vector<std::vector<uint8_t>> perms = Permutations::getPermutations(m + n);
}
