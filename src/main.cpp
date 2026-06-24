#include <iostream>
#include <vector>
#include <string>
// #include <sstream>
// #include <algorithm>
// #include <iomanip>
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


    // printf("Graph: \n");
    // graph.printGraph();

    // printf("Complete graph: \n");
    // cg.printGraph();


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
        // std::cout << "RUNNING IN DEBUG MODE" << std::endl;
    #else
        // std::cout << "RUNNING IN RELEASE MODE" << std::endl;
    #endif

    std::vector<std::string> args(argv, argv + argc);

    size_t idx = 0;
    while(idx < args.size()){
        std::string arg = args[idx];

        if(arg == "-h" || arg == "--help"){
            printf("%s [OPTIONS...]\n", args[0].c_str());
            printf("\n");
            printf("Options:\n");
            printf("  -h --help                 Show this help\n");
            printf("  -p --print [PATTERN...]   Print adjacency matrix of the given g6 encoded graph\n");
            break;
        } else if(arg == "-p" || arg == "--print"){
            std::string encodedGraph = args[idx + 1];
            Graph<>::printGraph(encodedGraph);
            break;
        }

        ++idx;
    }


    // if(argc < 2){
    //     std::cerr << "Usage: " << argv[0] << " <graph_size>" << std::endl;
    //     return 1;
    // }

    // int graphSize = std::stoi(argv[1]);
    // if(graphSize < 1){
    //     std::cerr << "Error: Graph size must be between 1 and 20" << std::endl;
    //     return 1;
    // }

    // std::vector<int> pinnacleSet({ 3, 4, graphSize });

    // switch(graphSize){
    //     case 1:  runGraphPipeline<1>(pinnacleSet);  break;
    //     case 2:  runGraphPipeline<2>(pinnacleSet);  break;
    //     case 3:  runGraphPipeline<3>(pinnacleSet);  break;
    //     case 4:  runGraphPipeline<4>(pinnacleSet);  break;
    //     case 5:  runGraphPipeline<5>(pinnacleSet);  break;
    //     case 6:  runGraphPipeline<6>(pinnacleSet);  break;
    //     case 7:  runGraphPipeline<7>(pinnacleSet);  break;
    //     case 8:  runGraphPipeline<8>(pinnacleSet);  break;
    //     case 9:  runGraphPipeline<9>(pinnacleSet);  break;
    //     case 10: runGraphPipeline<10>(pinnacleSet); break;
    //     case 11: runGraphPipeline<11>(pinnacleSet); break;
    //     case 12: runGraphPipeline<12>(pinnacleSet); break;
    //     case 13: runGraphPipeline<13>(pinnacleSet); break;
    //     case 14: runGraphPipeline<14>(pinnacleSet); break;
    //     case 15: runGraphPipeline<15>(pinnacleSet); break;
    //     case 16: runGraphPipeline<16>(pinnacleSet); break;
    //     case 17: runGraphPipeline<17>(pinnacleSet); break;
    //     case 18: runGraphPipeline<18>(pinnacleSet); break;
    //     case 19: runGraphPipeline<19>(pinnacleSet); break;
    //     case 20: runGraphPipeline<20>(pinnacleSet); break;
    // }

    // printf("Starting Graphs With 1 Vertex\n");
    // Graph<1>::getGraphStatsFast("../graphs/simple_connected_graphs/graph1c.g6");
    // std::cout << std::endl;

    // printf("Starting Graphs With 2 Vertices\n");
    // Graph<2>::getGraphStatsFast("../graphs/simple_connected_graphs/graph2c.g6");
    // std::cout << std::endl;

    // printf("Starting Graphs With 3 Vertices\n");
    // Graph<3>::getGraphStatsFast("../graphs/simple_connected_graphs/graph3c.g6");
    // std::cout << std::endl;

    // printf("Starting Graphs With 4 Vertices\n");
    // Graph<4>::getGraphStatsFast("../graphs/simple_connected_graphs/graph4c.g6");
    // std::cout << std::endl;

    // printf("Starting Graphs With 5 Vertices\n");
    // Graph<5>::getGraphStatsFast("../graphs/simple_connected_graphs/graph5c.g6");
    // std::cout << std::endl;

    // printf("Starting Graphs With 6 Vertices\n");
    // Graph<6>::getGraphStatsFast("../graphs/simple_connected_graphs/graph6c.g6");
    // std::cout << std::endl;

    // printf("Starting Graphs With 7 Vertices\n");
    // Graph<7>::getGraphStatsFast("../graphs/simple_connected_graphs/graph7c.g6");
    // std::cout << std::endl;

    // printf("Graphs with 8 vertices\n");
    // Graph<8>::getGraphStatsFast("../graphs/simple_connected_graphs/graph8c.g6");

    // printf("Starting Graphs With 9 Vertices\n");
    // auto startFast = std::chrono::high_resolution_clock::now();
    // Graph<9>::getGraphStatsFast("../graphs/simple_connected_graphs/graph9c.g6");
    // auto endFast = std::chrono::high_resolution_clock::now();
    // auto fastDiff = duration(startFast, endFast) / 1000;

    // printf("Fast Version: %fs\n", fastDiff);

    return 0;
}
