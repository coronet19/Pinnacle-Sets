#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <algorithm>
#include <iomanip>

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

int calculateFreeLabels(int m, int n, int k){
    if(m == k){
        return n - 1;
    } else{
        return m + n - k - 1;
    }
}

int calculateLabelings(int m, int n, int k){
    int f = calculateFreeLabels(m, n, k);
    int res;

    if(k > n){
        res = n * nPr(m, k) * factorial(f);
        // res = nPr(m, k) * nCr(n, n - 1) * factorial(f);
        // res = nPr(m, k) * factorial(n) * factorial(f);
    } else if(k == n){
        res = n * nPr(m, k) * factorial(f) + factorial(m) * factorial(n);
        // res = nPr(m, k) * nCr(n, n - 1) * factorial(f) + factorial(m) * factorial(n);
        // res = nPr(m, k) * factorial(f) + factorial(m) * factorial(n) * 2;
    } else{
        // int lesser = calculateLabelings(m, n, n);// + factorial(m);
        // res = pow(2, n - k) * lesser + pow(2, n - k - 1) * factorial(m + 1);
        res = -1;
    }

    return res;
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
    int iterations = 0;
    int totalPerms = factorial(m + n);

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

        int r = count / (factorial(m) * factorial(n));
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
        << nCr(m + n, std::max(m, n))
        << std::endl;


    return 0;
}
