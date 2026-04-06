#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <algorithm>
#include <iomanip>

#include "../include/permutations.h"



int main(int argc, char** argv){
    int m, n;
    std::string nums;

    std::cout << "Enter m: ";
    std::cin >> m;

    std::cout << "Enter n: ";
    std::cin >> n;

    // std::vector<std::vector<uint8_t>> pinnacleSets = Permutations::createPinnacleSets(m, n);
    std::vector<std::vector<uint8_t>> pinnacleSets = Permutations::createBipartitePinnacleSets(m, n);
    // std::vector<std::vector<uint8_t>> perms = Permutations::getPermutations(m + n);
    std::vector<int> counts(pinnacleSets.size(), 0);
    int total = 0;
    int iterations = 0;
    int totalPerms = Permutations::factorial(m + n);

    std::vector<uint8_t> perm;
    for(int i = 1; i <= m + n; ++i){
        perm.push_back(i);
    }

    do{
        for(int i = 0; i < pinnacleSets.size(); ++i){
            counts[i] += Permutations::isValidLabeling(perm, pinnacleSets[i], m, n);
        }

        ++iterations;
        if(iterations % 10000 == 0){
            float f = 100 * (float)iterations / (float)totalPerms;
            printf("Permutations Checked: %d / %d, ", iterations, totalPerms);
            std::cout << std::fixed << std::setprecision(2) << f << "%" << std::endl;
            // std::cout << "Permutations Checked: " << iterations << std::endl;
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

        int r = count / (Permutations::factorial(m) * Permutations::factorial(n));
        printf("Valid Labelings: %d = %d! * %d! * %d!\n", count, m, n, r);

        if(m > 0 && n > 0 && pinnacleSet.size() > 0){
            int predLabels = calculateLabelings(m, n, pinnacleSet.size());
            if(predLabels == -1) continue;
            // printf("m = %d, n = %d, k = %d\n", m, n, pinnacleSet.size());
            // int predLabels = calculateLabelings(m, n, pinnacleSet.size());
            printf("Predicted Labels: %d\n", predLabels);
        }
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
        << Permutations::nCr(m + n, std::max(m, n))
        << std::endl;


    return 0;
}
