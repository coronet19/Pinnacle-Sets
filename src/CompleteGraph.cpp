#include "../include/CompleteGraph.h"


// Gets the next lexicographic pinnacle set that
// maintains strictly increasing ordering
bool CompleteGraph::getNextPinnacleSet(std::vector<int>& p){
    if(p.size() < 2){
        return false;
    }

    int k = static_cast<int>(p.size());
    int targetIdx = -1;

    for(int i = k - 2; i >= 0; --i){
        int ceiling = this->size - (k - 1 - i);
        if(p[i] < ceiling){
            targetIdx = i;
            break;
        }
    }

    if(targetIdx == -1){
        return false;
    }

    p[targetIdx]++;

    for(int j = targetIdx + 1; j < k - 1; ++j){
        p[j] = p[j - 1] + 1;
    }

    return true;
}

// get all admissable pinnacle sets for the graph structure edgeMap
std::vector<std::vector<int>> CompleteGraph::getAdmissablePinnacleSets(){
    std::vector<std::vector<int>> validLabelings;

    // 1. Ensure graph and pinnacles are sorted to start at the first lexicographical permutation
    std::sort(this->graph.begin(), this->graph.end());
    std::sort(this->pinnacles.begin(), this->pinnacles.end());

    // 2. Iterate through EVERY combination of the pinnacles
    do{
        // 3. Iterate through EVERY permutation of the labels (1, 2, 3, ..., size)
        do{
            // 4. Check if the current permutation of graph values
            // makes the pinnacles set valid for this edgeMap
            if(this->isValidLabeling(this->pinnacles)){
                validLabelings.push_back(this->pinnacles);
            }
        } while(std::next_permutation(this->graph.begin(), this->graph.end()));
    } while(CompleteGraph::getNextPinnacleSet(this->pinnacles));


    return validLabelings;
}

bool CompleteGraph::isValidLabeling(const std::vector<int>& pinnacleSet){
    if(edgeMap.size() != this->size){
        return false;
    }

    for(int vertexIdx = 0; vertexIdx < this->edgeMap.size(); ++vertexIdx){
        std::vector<int>* neighbors = &(edgeMap[vertexIdx]);
        int vertexValue = this->graph[vertexIdx];
        bool shouldBePinnacle = std::binary_search(pinnacleSet.begin(), pinnacleSet.end(), vertexValue);
        bool actsAsPinnacle = !neighbors->empty();

        for(int edgeIdx : *neighbors){
            if(vertexValue <= this->graph[edgeIdx]){
                actsAsPinnacle = false;
                break;
            }
        }

        if(actsAsPinnacle != shouldBePinnacle){
            return false;
        }
    }
    return true;
}
