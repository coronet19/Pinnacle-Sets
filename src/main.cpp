#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <algorithm>
#include <iomanip>

#include "../include/permutations.h"
#include "../include/CompleteGraph.h"


int main(int argc, char** argv){
    #ifdef _DEBUG
        std::cout << "RUNNING IN DEBUG MODE" << std::endl;
    #else
        std::cout << "RUNNING IN RELEASE MODE" << std::endl;
    #endif

    int graphSize = 7;
    std::vector<int> pinnacleSet({ 3, graphSize });
    CompleteGraph g(graphSize, pinnacleSet);
    std::vector<std::vector<int>> admissablePinnacleSets = g.getAdmissablePinnacleSets();
    std::map<std::vector<int>, int> labelingsPerPinnacle;

    for(const auto& p : admissablePinnacleSets){
        labelingsPerPinnacle[p]++;
    }

    printf("Admissable Pinnacle Sets: %d\n", (int)admissablePinnacleSets.size());



    for(const auto& [p, count] : labelingsPerPinnacle){
        // int predictedLabelings = calculatePredictedLabelings(graphSize, p);

        if(p == pinnacleSet){
            printf("Initial Pinnacle Set: { ");
        } else{
            printf("Pinnacle Set: { ");
        }

        for(int i = 0; i < p.size(); ++i){
            printf("%d", p[i]);
            if(i < p.size() - 1) printf(", ");
        }
        printf(" }, Labelings: %d\n", count);
        // printf(" }, Labelings: %d, Predicted Labelings: %d\n", count, predictedLabelings);


    }


    return 0;
}
