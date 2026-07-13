// #include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <filesystem>
#include <chrono>
#include <unistd.h>
#include <cinttypes>

// #include "../include/permutations.h"
#include "graph.hpp"
#include "tui.hpp"


// Simple helper to get the current time
auto now() { return std::chrono::high_resolution_clock::now(); }

// Helper to calculate duration in milliseconds (or microseconds)
double duration(std::chrono::high_resolution_clock::time_point start, std::chrono::high_resolution_clock::time_point end) {
    return std::chrono::duration<double, std::milli>(end - start).count();
}

// Parses "N" or "N-M" into a [lo, hi] range. Returns false on any error.
static bool parseRange(const std::string& s, size_t& lo, size_t& hi) {
    auto dash = s.find('-');
    try {
        if (dash == std::string::npos) {
            lo = hi = std::stoul(s);
        } else {
            lo = std::stoul(s.substr(0, dash));
            hi = std::stoul(s.substr(dash + 1));
        }
    } catch (...) {
        return false;
    }
    return lo >= 1 && hi <= 10 && lo <= hi;
}

static void runStats(size_t lo, size_t hi, bool force, bool extra, bool useTui) {
    for (size_t n = lo; n <= hi; ++n) {
        std::string path = "../graphs/simple_connected_graphs/graphs/graph"
                         + std::to_string(n) + "c.g6";
        std::string statsPath = "../graphs/simple_connected_graphs/stats/graph"
                              + std::to_string(n) + "c_stats.csv";
        std::string extraPath = "../graphs/simple_connected_graphs/stats/graph"
                              + std::to_string(n) + "c_stats_extra.csv";

        if (force && std::filesystem::exists(statsPath)) {
            std::filesystem::remove(statsPath);
            printf("Removed existing %s\n", statsPath.c_str());

            if(extra){
                std::filesystem::remove(extraPath);
                printf("Removed existing %s\n", extraPath.c_str());
            }
        }

        auto t0 = std::chrono::high_resolution_clock::now();
        if (useTui) {
            ProgressState progress;
            Tui tui(progress);
            tui.start();
            Graph::getGraphStatsFast(n, path, &progress, extra);
            tui.stop();
        } else {
            printf("Starting graphs with %zu vertices\n", n);
            Graph::getGraphStatsFast(n, path, nullptr, extra);
        }
        auto t1 = std::chrono::high_resolution_clock::now();

        double secs = std::chrono::duration<double>(t1 - t0).count();
        printf("Done with size %zu in %.3fs\n\n", n, secs);
    }
}

// pinnacle set : { low, low + 1, ... , graphSize - 1, graphSize }
// static void computePathGraphStats(size_t graphSize){
//     for(size_t size = 1; size <= graphSize; ++size){
//         Graph g(size, Graph::makePathGraph(size));

//         printf("Path graph P_%zu:\n", size);

//         // Contiguous-run pinnacle sets { low, low+1, ..., size } with low > 1.
//         // (A single-vertex path has no such set, so nothing is printed for size 1.)
//         for(size_t low = size / 2 + 1; low <= size; ++low){
//             std::vector<int> pinnacleSet;
//             pinnacleSet.reserve(size - low + 1);
//             for(size_t v = low; v <= size; ++v)
//                 pinnacleSet.push_back(static_cast<int>(v));

//             uint64_t count = 0;
//             g.countHeapPermutations(pinnacleSet, static_cast<int>(size), count);

//             printf("  { %zu, ..., %zu }: %lu = %d! * %lld valid labeling(s)\n", low, size, count, (int)(size - low + 1), count / Permutations::factorial(size - low + 1));
//         }
//     }
// }

// pinnacle set : { low, low + 1, ... , graphSize - 1, graphSize }
static void computePathGraphStats(size_t graphSize){
    if(graphSize < 2)
        return;
    const size_t maxK = (graphSize + 1) / 2;  // ceil(graphSize / 2), the largest possible pinnacle set

    // counts[n][k] = valid labelings of P_n with contiguous pinnacle set { n-k+1, ..., n }.
    // Cells with k > ceil(n/2) stay 0, matching the zero entries in the paper's table.
    std::vector<std::vector<uint64_t>> counts(graphSize + 1, std::vector<uint64_t>(maxK + 1, 0));
    for(size_t n = 2; n <= graphSize; ++n){
        Graph g(n, Graph::makePathGraph(n));
        for(size_t k = 1; k <= (n + 1) / 2; ++k){
            std::vector<int> pinnacleSet;
            pinnacleSet.reserve(k);
            for(size_t v = n - k + 1; v <= n; ++v)
                pinnacleSet.push_back(static_cast<int>(v));
            g.countHeapPermutations(pinnacleSet, static_cast<int>(n), counts[n][k]);
        }
    }

    // Column widths: label column fits "n\k" and the largest n;
    // each k column fits its header and its widest count.
    size_t labelWidth = std::max(std::string("n\\k").size(), std::to_string(graphSize).size());
    std::vector<size_t> width(maxK + 1, 0);
    for(size_t k = 1; k <= maxK; ++k){
        width[k] = std::to_string(k).size();
        for(size_t n = 2; n <= graphSize; ++n)
            width[k] = std::max(width[k], std::to_string(counts[n][k]).size());
    }

    auto printRule = [&](char fill){
        printf("+%s+", std::string(labelWidth + 2, fill).c_str());  // extra '+' makes the "++" double bar
        for(size_t k = 1; k <= maxK; ++k)
            printf("+%s", std::string(width[k] + 2, fill).c_str());
        printf("+\n");
    };

    printRule('-');
    printf("| %*s |", (int)labelWidth, "n\\k");  // trailing '|' pairs with the next cell's '|' -> "||"
    for(size_t k = 1; k <= maxK; ++k)
        printf("| %*zu ", (int)width[k], k);
    printf("|\n");
    printRule('=');                              // double rule under the header, like \hline\hline

    for(size_t n = 2; n <= graphSize; ++n){
        printf("| %*zu |", (int)labelWidth, n);
        for(size_t k = 1; k <= maxK; ++k)
            printf("| %*" PRIu64 " ", (int)width[k], counts[n][k]);
        printf("|\n");
        printRule('-');
    }
}


// pinnacle set : { low, low + 1, ... , graphSize - 1, graphSize }
static void computeCycleGraphStats(size_t graphSize){
    if(graphSize < 3)
        return;
    const size_t maxK = graphSize / 2;  // floor(graphSize / 2), the largest possible pinnacle set on a cycle

    // counts[n][k] = valid labelings of C_n with contiguous pinnacle set { n-k+1, ..., n }.
    // Cells with k > floor(n/2) stay 0 (a cycle's independence number is floor(n/2)).
    std::vector<std::vector<uint64_t>> counts(graphSize + 1, std::vector<uint64_t>(maxK + 1, 0));
    for(size_t n = 3; n <= graphSize; ++n){
        Graph g(n, Graph::makeCycleGraph(n));
        for(size_t k = 1; k <= n / 2; ++k){
            std::vector<int> pinnacleSet;
            pinnacleSet.reserve(k);
            for(size_t v = n - k + 1; v <= n; ++v)
                pinnacleSet.push_back(static_cast<int>(v));
            g.countHeapPermutations(pinnacleSet, static_cast<int>(n), counts[n][k]);
        }
    }

    // Column widths: label column fits "n\k" and the largest n;
    // each k column fits its header and its widest count.
    size_t labelWidth = std::max(std::string("n\\k").size(), std::to_string(graphSize).size());
    std::vector<size_t> width(maxK + 1, 0);
    for(size_t k = 1; k <= maxK; ++k){
        width[k] = std::to_string(k).size();
        for(size_t n = 3; n <= graphSize; ++n)
            width[k] = std::max(width[k], std::to_string(counts[n][k]).size());
    }

    auto printRule = [&](char fill){
        printf("+%s+", std::string(labelWidth + 2, fill).c_str());  // extra '+' makes the "++" double bar
        for(size_t k = 1; k <= maxK; ++k)
            printf("+%s", std::string(width[k] + 2, fill).c_str());
        printf("+\n");
    };

    printRule('-');
    printf("| %*s |", (int)labelWidth, "n\\k");  // trailing '|' pairs with the next cell's '|' -> "||"
    for(size_t k = 1; k <= maxK; ++k)
        printf("| %*zu ", (int)width[k], k);
    printf("|\n");
    printRule('=');                              // double rule under the header, like \hline\hline

    for(size_t n = 3; n <= graphSize; ++n){
        printf("| %*zu |", (int)labelWidth, n);
        for(size_t k = 1; k <= maxK; ++k)
            printf("| %*" PRIu64 " ", (int)width[k], counts[n][k]);
        printf("|\n");
        printRule('-');
    }
}


int main(int argc, char** argv){
    std::vector<std::string> args(argv, argv + argc);

    size_t idx = 1;
    while(idx < args.size()){
        std::string arg = args[idx];

        if(arg == "-h" || arg == "--help"){
            printf("Usage: %s [OPTIONS...]\n\n", args[0].c_str());
            printf("Options:\n");
            printf("  -h  --help               Show this help\n");
            printf("  -p  --print <g6>         Print adjacency matrix of the given g6 encoded graph\n");
            printf("  -pg --path_graph <size>  Print stats for all path graphs of size n <= <size>\n");
            printf("  -cg --cycle_graph <size> Print stats for all cycle graphs of size n <= <size>\n");
            printf("  -s  --stats <range>      Compute edge-removal stats for graphs of the given size(s).\n");
            printf("                           <range> is N or N-M where 1 <= N <= M <= 9\n");
            printf("                           Output is written to graphs/simple_connected_graphs/graphNc_stats.csv\n");
            printf("      --force              Delete existing stats file(s) and recalculate from scratch\n");
            printf("      --extra              Logs extra info about each pinnacle set\n");
            printf("      --no-tui             Disable the live progress display (plain log output)\n");
            return 0;
        } else if(arg == "-p" || arg == "--print"){
            if (idx + 1 >= args.size()) {
                fprintf(stderr, "Error: --print requires a g6 encoded graph argument\n");
                return 1;
            }
            Graph::printGraph(args[idx + 1]);
            return 0;
        } else if(arg == "-pg" || arg == "--path_graph"){
            if(idx + 1 >= args.size()) {
                fprintf(stderr, "Error: --path_graph requires a graph size\n");
                return 1;
            }

            size_t graphSize = -1;
            try {
                graphSize = std::stoul(args[idx + 1]);
            } catch (...) {
                // do nothing
            }

            if(graphSize < 1){
                fprintf(stderr, "Error: invalid size '%s' - expected N where N > 1 \n", args[idx + 1].c_str());
                return 1;
            }

            computePathGraphStats(graphSize);

            return 0;
        } else if(arg == "-cg" || arg == "--cycle_graph"){
            if(idx + 1 >= args.size()) {
                fprintf(stderr, "Error: --cycle_graph requires a graph size\n");
                return 1;
            }

            size_t graphSize = -1;
            try {
                graphSize = std::stoul(args[idx + 1]);
            } catch (...) {
                // do nothing
            }

            if(graphSize < 1){
                fprintf(stderr, "Error: invalid size '%s' - expected N where N > 1 \n", args[idx + 1].c_str());
                return 1;
            }

            computeCycleGraphStats(graphSize);

            return 0;
        } else if(arg == "-s" || arg == "--stats"){
            if (idx + 1 >= args.size()) {
                fprintf(stderr, "Error: --stats requires a range argument (e.g. 5 or 1-9)\n");
                return 1;
            }
            size_t lo, hi;
            if (!parseRange(args[idx + 1], lo, hi)) {
                fprintf(stderr, "Error: invalid range '%s' — expected N or N-M where 1 <= N <= M <= 9\n",
                        args[idx + 1].c_str());
                return 1;
            }
            bool force = std::find(args.begin(), args.end(), "--force") != args.end();
            bool extra = std::find(args.begin(), args.end(), "--extra") != args.end();
            bool noTui = std::find(args.begin(), args.end(), "--no-tui") != args.end();
            // Use the live display only on an interactive terminal (and unless disabled),
            // so redirecting output to a file or pipe yields clean, escape-free logs.
            bool useTui = !noTui && isatty(STDOUT_FILENO);
            runStats(lo, hi, force, extra, useTui);
            return 0;
        }

        ++idx;
    }

    if (args.size() == 1) {
        printf("Usage: %s [OPTIONS...]\nRun with --help for usage information.\n", args[0].c_str());
    }

    return 0;
}
