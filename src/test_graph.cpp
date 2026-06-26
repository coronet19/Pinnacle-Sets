#include <iostream>
#include <vector>
#include <algorithm>
#include "graph.hpp"

static int passed = 0, failed = 0;

void check(bool cond, const char* name) {
    if (cond) { printf("  PASS: %s\n", name); ++passed; }
    else       { printf("  FAIL: %s\n", name); ++failed; }
}

// P4: path graph  0 - 1 - 2 - 3
Graph makeP4() {
    // adj[v] has bit j set iff edge (v, j) exists
    std::vector<uint64_t> adj = {
        0b0010,  // vertex 0: neighbor 1
        0b0101,  // vertex 1: neighbors 0, 2
        0b1010,  // vertex 2: neighbors 1, 3
        0b0100,  // vertex 3: neighbor 2
    };
    return Graph(4, adj);
}

// P3: path graph  0 - 1 - 2
Graph makeP3() {
    std::vector<uint64_t> adj = {
        0b010,  // vertex 0: neighbor 1
        0b101,  // vertex 1: neighbors 0, 2
        0b010,  // vertex 2: neighbor 1
    };
    return Graph(3, adj);
}

// K3: complete graph on 3 vertices (every pair connected)
Graph makeK3() {
    std::vector<uint64_t> adj = {
        0b110,  // vertex 0: neighbors 1, 2
        0b101,  // vertex 1: neighbors 0, 2
        0b011,  // vertex 2: neighbors 0, 1
    };
    return Graph(3, adj);
}

// ---- Tests ----------------------------------------------------------------

void testFullMask() {
    printf("testFullMask:\n");
    auto g = makeP4();
    check(g.fullMask() == 0xFULL,  "n=4 → fullMask = 0b1111");
    Graph g1(1, {0});
    check(g1.fullMask() == 0x1ULL, "n=1 → fullMask = 0b0001");
    Graph g3 = makeK3();
    check(g3.fullMask() == 0x7ULL, "n=3 → fullMask = 0b0111");
}

void testResetValues() {
    printf("testResetValues:\n");
    auto g = makeP4();
    check(g.values == std::vector<int>({1,2,3,4}), "initial values are {1,2,3,4}");
    g.values[0] = 99;
    g.resetValues();
    check(g.values == std::vector<int>({1,2,3,4}), "resetValues restores {1,2,3,4}");
}

void testIsConnected() {
    printf("testIsConnected:\n");
    check(makeP4().isConnected(),  "P4 is connected");
    check(makeP3().isConnected(),  "P3 is connected");
    check(makeK3().isConnected(),  "K3 is connected");

    // Two isolated vertices (no edges)
    Graph disc(2, {0, 0});
    check(!disc.isConnected(), "two isolated vertices are disconnected");

    // Single vertex
    check(Graph(1, {0}).isConnected(), "single vertex is connected");

    // Path 0-1 with a dangling isolated vertex 2
    Graph halfDisc(3, {0b010, 0b001, 0b000});
    check(!halfDisc.isConnected(), "path + isolated vertex is disconnected");
}

void testGetIndependentSets() {
    printf("testGetIndependentSets:\n");
    auto p4 = makeP4();

    // P4 independent sets of size 2: {0,2}, {0,3}, {1,3}
    auto sets2 = p4.getIndependentSets(2);
    check(sets2.size() == 3, "P4 has exactly 3 independent sets of size 2");

    bool has02 = false, has03 = false, has13 = false;
    for (const auto& s : sets2) {
        if (s == std::vector<int>({0,2})) has02 = true;
        if (s == std::vector<int>({0,3})) has03 = true;
        if (s == std::vector<int>({1,3})) has13 = true;
    }
    check(has02 && has03 && has13, "P4 independent sets of size 2 are {0,2}, {0,3}, {1,3}");

    // Every vertex is its own independent set of size 1
    check(p4.getIndependentSets(1).size() == 4, "P4 has 4 independent sets of size 1");

    // K3 has no independent set of size 2 (every pair of vertices is adjacent)
    check(makeK3().getIndependentSets(2).empty(), "K3 has no independent sets of size 2");
}

void testIsValidLabeling() {
    printf("testIsValidLabeling:\n");
    auto p4 = makeP4();

    // values = {1,2,4,3}: vertex 2 carries label 4 (max), its neighbors have labels 2 and 3
    //   → vertex 2 is the only pinnacle → pinnacle set = {4}
    p4.values = {1,2,4,3};
    check( p4.isValidLabeling({4}),   "P4 {1,2,4,3}: pinnacle set is {4}");
    check(!p4.isValidLabeling({3}),   "P4 {1,2,4,3}: pinnacle set is NOT {3}");
    check(!p4.isValidLabeling({3,4}), "P4 {1,2,4,3}: pinnacle set is NOT {3,4}");

    // values = {4,1,3,2}:
    //   vertex 0 (label 4): neighbor vertex 1 (label 1) → 4 > 1 → pinnacle
    //   vertex 2 (label 3): neighbors vertex 1 (label 1) and vertex 3 (label 2) → 3 > both → pinnacle
    //   → pinnacle set = {3,4}
    p4.values = {4,1,3,2};
    check( p4.isValidLabeling({3,4}), "P4 {4,1,3,2}: pinnacle set is {3,4}");
    check(!p4.isValidLabeling({4}),   "P4 {4,1,3,2}: pinnacle set is NOT {4}");

    // pinnacleSet must end with graphSize — anything else is immediately invalid
    check(!p4.isValidLabeling({3}), "pinnacle set not ending in graphSize is invalid");
    check(!p4.isValidLabeling({}),  "empty pinnacle set is invalid");
}

void testCountHeapPermutations() {
    printf("testCountHeapPermutations:\n");

    // K3: in a complete graph every labeling has exactly one pinnacle — the vertex with the max label.
    // So all 3! = 6 labelings have pinnacle set {3}.
    auto k3 = makeK3();
    int count = 0;
    k3.resetValues();
    k3.countHeapPermutations(std::vector<int>{3}, 3, count);
    check(count == 6, "K3: all 6 labelings have pinnacle set {3}");

    // P3 with pinnacle set {3}:
    //   3 at vertex 1 (interior): both endpoints have label < 3 and are not pinnacles → 2 valid labelings
    //   3 at vertex 0 (endpoint): only the arrangement where vertex 1 < vertex 2 avoids a 2nd pinnacle → 1
    //   3 at vertex 2 (endpoint): symmetric to vertex 0 case → 1
    //   Total = 4
    auto p3 = makeP3();
    count = 0;
    p3.resetValues();
    p3.countHeapPermutations(std::vector<int>{3}, 3, count);
    check(count == 4, "P3: 4 labelings have pinnacle set {3}");

    // P4 with pinnacle set {4}:
    //   Label 4 at vertex 1 or vertex 2 (interior vertices) ensures a single pinnacle.
    //   Enumerate: 4 at vertex 1 → neighbors (0,2) get any subset of {1,2,3} with vertex 3 getting
    //   the remainder. Vertex 1 is a pinnacle. Vertex 3 is a pinnacle only if label[3] > label[2].
    //   Careful enumeration gives 8 valid labelings.
    auto p4 = makeP4();
    count = 0;
    p4.resetValues();
    p4.countHeapPermutations(std::vector<int>{4}, 4, count);
    check(count == 8, "P4: 8 labelings have pinnacle set {4}");
}

void testDecodeGraphG6() {
    printf("testDecodeGraphG6:\n");

    // Single vertex: '@' (ASCII 64, 64-63=1 vertex, no edge bytes)
    auto single = Graph::decodeGraphG6(1, "@");
    check(single.size() == 1 && single[0] == 0, "single-vertex g6 '@': no edges");

    // K3: 'B' (3 vertices) + 'w' (edges 0-1, 0-2, 1-2 → bits 111 in MSB order → 56 → 56+63=119='w')
    auto k3adj = Graph::decodeGraphG6(3, "Bw");
    check(k3adj.size() == 3, "K3 g6 'Bw': 3 rows in adjacency matrix");
    check(k3adj[0] == 0b110 && k3adj[1] == 0b101 && k3adj[2] == 0b011,
          "K3 g6 'Bw': adjacency matrix correct");

    // Verify the decoded K3 is actually connected and has no size-2 independent sets
    Graph k3decoded(3, k3adj);
    check(k3decoded.isConnected(), "K3 decoded from g6 is connected");
    check(k3decoded.getIndependentSets(2).empty(), "K3 decoded from g6 has no size-2 independent sets");
}

void testGetNextPinnacleSet() {
    printf("testGetNextPinnacleSet:\n");

    // For n=4, size-2 pinnacle sets in lex order: {2,4} → {3,4} → (done)
    std::vector<int> p = {2, 4};
    check( Graph::getNextPinnacleSet(p, 4), "{2,4} has a successor");
    check(p == std::vector<int>({3,4}),     "{2,4} → {3,4}");
    check(!Graph::getNextPinnacleSet(p, 4), "{3,4} has no successor (last of its size for n=4)");

    // For n=4, size-1 pinnacle sets: only {4}
    std::vector<int> q = {4};
    check(!Graph::getNextPinnacleSet(q, 4), "size-1 pinnacle set {4} has no successor");

    // For n=5, size-2: {2,5} → {3,5} → {4,5} → (done)
    std::vector<int> r = {2, 5};
    Graph::getNextPinnacleSet(r, 5);
    Graph::getNextPinnacleSet(r, 5);
    check(r == std::vector<int>({4,5}),     "n=5 size-2 sequence ends at {4,5}");
    check(!Graph::getNextPinnacleSet(r, 5), "{4,5} has no successor for n=5");
}

void testGeneratePinnacleSets() {
    printf("testGeneratePinnacleSets:\n");

    // n=4: base sets are {4}, {2,4}, {2,3,4}
    auto sets4 = Graph::generatePinnacleSets(4);
    check(sets4.size() == 3,                         "n=4 generates 3 base pinnacle sets");
    check(sets4[0] == std::vector<int>({4}),         "first base set = {4}");
    check(sets4[1] == std::vector<int>({2,4}),       "second base set = {2,4}");
    check(sets4[2] == std::vector<int>({2,3,4}),     "third base set = {2,3,4}");

    // n=3: base sets are {3}, {2,3}
    auto sets3 = Graph::generatePinnacleSets(3);
    check(sets3.size() == 2,                         "n=3 generates 2 base pinnacle sets");
    check(sets3[0] == std::vector<int>({3}),         "n=3 first base set = {3}");
    check(sets3[1] == std::vector<int>({2,3}),       "n=3 second base set = {2,3}");
}

// ---- Main -----------------------------------------------------------------

int main() {
    testFullMask();
    testResetValues();
    testIsConnected();
    testGetIndependentSets();
    testIsValidLabeling();
    testCountHeapPermutations();
    testDecodeGraphG6();
    testGetNextPinnacleSet();
    testGeneratePinnacleSets();

    printf("\n%d passed, %d failed\n", passed, failed);
    return failed > 0 ? 1 : 0;
}
