#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <algorithm>
#include <iomanip>

#include "../include/permutations.h"
#include "../include/CompleteGraph.h"



int main(int argc, char** argv){
    int graphSize = 7;
    std::vector<int> pinnacleSet({ 3, 6, graphSize });
    CompleteGraph g(graphSize, pinnacleSet);
    std::vector<std::vector<int>> admissablePinnacleSets = g.getAdmissablePinnacleSets();
    std::map<std::vector<int>, int> labelingsPerPinnacle;

    for(const auto& p : admissablePinnacleSets){
        labelingsPerPinnacle[p]++;
    }

    printf("Admissable Pinnacle Sets: %d\n", (int)admissablePinnacleSets.size());

    for(const auto& [p, count] : labelingsPerPinnacle){
        printf("Pinnacle Set: { ");
        for(int i = 0; i < p.size(); ++i){
            printf("%d", p[i]);
            if(i < p.size() - 1) printf(", ");
        }
        printf(" }, Labelings: %d\n", count);


    }


    return 0;
}
