#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <algorithm>
#include <iomanip>

#include "../include/permutations.h"
#include "../include/CompleteGraph.h"
#include "graph.hpp"


int main(int argc, char** argv){
    #ifdef _DEBUG
        std::cout << "RUNNING IN DEBUG MODE" << std::endl;
    #else
        std::cout << "RUNNING IN RELEASE MODE" << std::endl;
    #endif

    const int graphSize = 8;
    std::vector<int> pinnacleSet({ 3, 4, graphSize });
    CompleteGraph cg(graphSize, pinnacleSet);
    std::vector<std::vector<int>> admissablePinnacleSets = cg.getAdmissablePinnacleSets();
    std::map<std::vector<int>, int> labelingsPerPinnacle;

    for(const auto& p : admissablePinnacleSets){
        labelingsPerPinnacle[p]++;
    }

    auto adjMatrix = Graph<graphSize>::makeCompleteGraph(pinnacleSet);
    Graph<graphSize> graph(adjMatrix);

    int numValid = 0;
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
        // printf(" }, Labelings: %d, Predicted Labelings: %d\n", count, predictedLabelings);

        graph.resetValues();

        int n = 0;
        do{
            n += graph.isValidLabeling(p);
        } while(std::next_permutation(graph.values.begin(), graph.values.end()));


        printf(" }, Complete Graph Labelings: %d, Graph Labelings: %d\n", count, n);

        numValid += n;
    }

    printf("Admissable Pinnacle Sets According To Graph.hpp: %d\n", numValid);

    return 0;
}
