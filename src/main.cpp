#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <algorithm>
#include <iomanip>
#include <chrono>

#include "../include/CompleteGraph.h"
#include "graph.hpp"


// Simple helper to get the current time
auto now() { return std::chrono::high_resolution_clock::now(); }

// Helper to calculate duration in milliseconds (or microseconds)
double duration(std::chrono::high_resolution_clock::time_point start, std::chrono::high_resolution_clock::time_point end) {
    return std::chrono::duration<double, std::milli>(end - start).count();
}


int main(int argc, char** argv){
    #ifdef _DEBUG
        std::cout << "RUNNING IN DEBUG MODE" << std::endl;
    #else
        std::cout << "RUNNING IN RELEASE MODE" << std::endl;
    #endif

    const int graphSize = 10;
    std::vector<int> pinnacleSet({ 3, 4, graphSize });


    auto adjMatrix = Graph<graphSize>::makeCompleteGraph(pinnacleSet);
    Graph<graphSize> graph(adjMatrix);

    CompleteGraph cg(graphSize, pinnacleSet);

    auto startComplete = std::chrono::high_resolution_clock::now();
    std::vector<std::vector<int>> admissableCompleteGraphPinnacleSets = cg.getAdmissablePinnacleSets();
    auto endComplete = std::chrono::high_resolution_clock::now();
    auto completeDiff = duration(startComplete, endComplete);

    std::vector<std::vector<int>> admissableGraphPinnacleSets = graph.getAdmissablePinnacleSets(pinnacleSet);

    std::map<std::vector<int>, int> labelingsPerPinnacle;

    for(const auto& p : admissableCompleteGraphPinnacleSets){
        labelingsPerPinnacle[p]++;
    }

    printf("cg size: %d\n", (int)labelingsPerPinnacle.size());
    printf("g size: %d\n", (int)admissableGraphPinnacleSets.size());


    printf("\n\n\n");

    double graphDiff = 0;
    int numValid = 0;
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

        auto startGraph = std::chrono::high_resolution_clock::now();

        int n = 0;
        graph.resetValues(); // Ensure values are {1, 2, ..., n}
        graph.countHeapPermutations(p, graphSize, n);

        auto endGraph = std::chrono::high_resolution_clock::now();
        graphDiff += duration(startGraph, endGraph);

        // printf(" }, Complete Graph Labelings: %d\n", count);
        printf(" }, Complete Graph Labelings: %d, Graph Labelings: %d\n", count, n);

        numValid += n;
    }

    // auto startGraph = std::chrono::high_resolution_clock::now();

    // graph.resetValues(); // Ensure values are {1, 2, ..., n}
    // graph.countHeapPermutations(admissableGraphPinnacleSets, graphSize, numValid);

    // auto endGraph = std::chrono::high_resolution_clock::now();
    // graphDiff += duration(startGraph, endGraph);


    printf("Admissable Pinnacle Sets According To CompleteGraph.cpp: %d\n", (int)admissableCompleteGraphPinnacleSets.size());
    printf("Admissable Pinnacle Sets According To Graph.hpp: %d\n", numValid);

    printf("CompleteGraph Duration: %f\n", completeDiff);
    printf("Graph Duration: %f\n", graphDiff);

    return 0;
}
