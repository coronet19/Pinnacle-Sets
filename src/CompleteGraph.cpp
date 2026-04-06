#include "../include/permutations.h"
#include "../include/CompleteGraph.h"


// Gets the next lexicographic pinnacle set that
// maintains strictly increasing ordering
bool CompleteGraph::getNextPinnacleSet(std::vector<int>& p){
    bool isLastPinnacleSet = true;
    int prev = p[0] - 1;
    for(const auto n : p){
        if(n != prev + 1){
            isLastPinnacleSet = false;
            break;
        } else{
            prev = n;
        }
    }

    if(isLastPinnacleSet) return false;


}

// get all admissable pinnacle sets for the graph structure edgeMap
std::vector<std::vector<int>> CompleteGraph::getAdmissablePinnacleSets(){
    std::vector<std::vector<int>> res;

    // 1 <= |p| < |graph|
    for(int pinnacleSetSize = 1; pinnacleSetSize < this->size; ++pinnacleSetSize){
        std::vector<int> p(pinnacleSetSize);

        for(int i = 0; i < pinnacleSetSize; ++i){
            p[i] = i + 1;
        }


        do{
            for(int i = 0; i < pinnacleSets.size(); ++i){
                counts[i] += Permutations::isValidLabeling(perm, pinnacleSets[i], m, n);
            }

            if(CompleteGraph::isValidLabeling(pinnacleSet)){
                ++count;
            }

            // ++iterations;
            // if(iterations % 10000 == 0){
            //     float f = 100 * (float)iterations / (float)totalPerms;
            //     printf("Permutations Checked: %d / %d, ", iterations, totalPerms);
            //     std::cout << std::fixed << std::setprecision(2) << f << "%" << std::endl;
            //     // std::cout << "Permutations Checked: " << iterations << std::endl;
            // }
        } while(CompleteGraph::getNextPinnacleSet(p));
    }

    return res;
}

bool CompleteGraph::isValidLabeling(const std::vector<int> pinnacleSet){
    for(const auto& pair : this->edgeMap){
        bool isPinnacle = true;
        int vertexIdx = pair.first;
        // std::vector<int>* edgeIdxs = &(pair.second);

        for(const auto& edgeIdx : pair.second){
            if(graph[vertexIdx] <= graph[edgeIdx]){ // should never be equal but we ball
                isPinnacle = false;
            }
        }

        auto it = std::find(pinnacleSet.begin(), pinnacleSet.end(), vertexIdx);
        bool shouldBePinnacle = it != pinnacleSet.end();
        if(isPinnacle != shouldBePinnacle) return false;
    }

    return true;
}
