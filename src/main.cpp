// #include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <filesystem>
#include <chrono>
#include <unistd.h>

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
        std::string path = "../graphs/simple_connected_graphs/graph"
                         + std::to_string(n) + "c.g6";
        std::string statsPath = "../graphs/simple_connected_graphs/graph"
                              + std::to_string(n) + "c_stats.csv";
        std::string extraPath = "../graphs/simple_connected_graphs/graph"
                              + std::to_string(n) + "c_stats_extra.csv";

        if (force && std::filesystem::exists(statsPath)) {
            std::filesystem::remove(statsPath);
            printf("Removed existing %s\n", statsPath.c_str());
        }
        if (force && std::filesystem::exists(extraPath)) {
            std::filesystem::remove(extraPath);
            printf("Removed existing %s\n", extraPath.c_str());
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

int main(int argc, char** argv){
    std::vector<std::string> args(argv, argv + argc);

    size_t idx = 1;
    while(idx < args.size()){
        std::string arg = args[idx];

        if(arg == "-h" || arg == "--help"){
            printf("Usage: %s [OPTIONS...]\n\n", args[0].c_str());
            printf("Options:\n");
            printf("  -h --help              Show this help\n");
            printf("  -p --print <g6>        Print adjacency matrix of the given g6 encoded graph\n");
            printf("  -s --stats <range>     Compute edge-removal stats for graphs of the given size(s).\n");
            printf("                         <range> is N or N-M where 1 <= N <= M <= 9\n");
            printf("                         Output is written to graphs/simple_connected_graphs/graphNc_stats.csv\n");
            printf("     --force             Delete existing stats file(s) and recalculate from scratch\n");
            printf("     --extra             Logs extra info about each pinnacle set\n");
            printf("     --no-tui            Disable the live progress display (plain log output)\n");
            return 0;
        } else if(arg == "-p" || arg == "--print"){
            if (idx + 1 >= args.size()) {
                fprintf(stderr, "Error: --print requires a g6 encoded graph argument\n");
                return 1;
            }
            Graph::printGraph(args[idx + 1]);
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
