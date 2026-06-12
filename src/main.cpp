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

// Helper function to handle the graph logic for a specific size
template<size_t graphSize>
void runGraphPipeline(const std::vector<int>& pinnacleSet) {
    auto adjMatrix = Graph<graphSize>::makeCompleteGraph(pinnacleSet);
    Graph<graphSize> graph(adjMatrix);

    CompleteGraph cg(graphSize, pinnacleSet);


    printf("Graph: \n");
    graph.printGraph();

    printf("Complete graph: \n");
    cg.printGraph();


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
}

int main(int argc, char** argv){
    #ifdef _DEBUG
        std::cout << "RUNNING IN DEBUG MODE" << std::endl;
    #else
        std::cout << "RUNNING IN RELEASE MODE" << std::endl;
    #endif

    if(argc < 2){
        std::cerr << "Usage: " << argv[0] << " <graph_size>" << std::endl;
        return 1;
    }

    int graphSize = std::stoi(argv[1]);
    if(graphSize < 1){
        std::cerr << "Error: Graph size must be between 1 and 20" << std::endl;
        return 1;
    }

    std::vector<int> pinnacleSet({ 3, 4, graphSize });

    switch(graphSize){
        case 1:  runGraphPipeline<1>(pinnacleSet);  break;
        case 2:  runGraphPipeline<2>(pinnacleSet);  break;
        case 3:  runGraphPipeline<3>(pinnacleSet);  break;
        case 4:  runGraphPipeline<4>(pinnacleSet);  break;
        case 5:  runGraphPipeline<5>(pinnacleSet);  break;
        case 6:  runGraphPipeline<6>(pinnacleSet);  break;
        case 7:  runGraphPipeline<7>(pinnacleSet);  break;
        case 8:  runGraphPipeline<8>(pinnacleSet);  break;
        case 9:  runGraphPipeline<9>(pinnacleSet);  break;
        case 10: runGraphPipeline<10>(pinnacleSet); break;
        case 11: runGraphPipeline<11>(pinnacleSet); break;
        case 12: runGraphPipeline<12>(pinnacleSet); break;
        case 13: runGraphPipeline<13>(pinnacleSet); break;
        case 14: runGraphPipeline<14>(pinnacleSet); break;
        case 15: runGraphPipeline<15>(pinnacleSet); break;
        case 16: runGraphPipeline<16>(pinnacleSet); break;
        case 17: runGraphPipeline<17>(pinnacleSet); break;
        case 18: runGraphPipeline<18>(pinnacleSet); break;
        case 19: runGraphPipeline<19>(pinnacleSet); break;
        case 20: runGraphPipeline<20>(pinnacleSet); break;
    }

    return 0;
}
