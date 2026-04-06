#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <algorithm>
#include <iomanip>

#include "../include/permutations.h"
#include "../include/CompleteGraph.h"

long long calculatePredictedLabelings(int n, const std::vector<int>& p) {
    long long res = Permutations::factorial(n - p.size());
    long long contiguousRunSum = 0;

    int last = -1;
    bool run = false;
    for(const auto& n : p){
        if(n == last + 1){
            contiguousRunSum++;
            if(run == false){
                contiguousRunSum++;
                run = true;
            }
        } else{
            run = false;
        }

        last = n;
    }

    if(contiguousRunSum > 0){
        res *= contiguousRunSum;
    }

    return res;
}

int main(int argc, char** argv){
    int graphSize = 9;
    std::vector<int> pinnacleSet({ 3, 4, 5, graphSize });
    CompleteGraph g(graphSize, pinnacleSet);
    std::vector<std::vector<int>> admissablePinnacleSets = g.getAdmissablePinnacleSets();
    std::map<std::vector<int>, int> labelingsPerPinnacle;

    for(const auto& p : admissablePinnacleSets){
        labelingsPerPinnacle[p]++;
    }

    printf("Admissable Pinnacle Sets: %d\n", (int)admissablePinnacleSets.size());



    for(const auto& [p, count] : labelingsPerPinnacle){
        int predictedLabelings = calculatePredictedLabelings(graphSize, p);

        if(p == pinnacleSet){
            printf("Initial Pinnacle Set: { ");
        } else{
            printf("Pinnacle Set: { ");
        }

        for(int i = 0; i < p.size(); ++i){
            printf("%d", p[i]);
            if(i < p.size() - 1) printf(", ");
        }
        printf(" }, Labelings: %d, Predicted Labelings: %d\n", count, predictedLabelings);


    }


    return 0;
}
