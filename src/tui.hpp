#pragma once

#include <atomic>
#include <chrono>
#include <cstdint>
#include <cstdio>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

// Shared, thread-safe progress state for the stats analysis.
//
// Worker threads update the counters and the "current" graph fields as they
// pick up work; a single Tui renderer thread reads a consistent snapshot each
// frame and draws it. Counters are atomics (cheap, lock-free); the string
// fields are guarded by a mutex since std::string isn't atomically assignable.
struct ProgressState {
    std::atomic<size_t> processed{0};   // graphs finished so far (incl. resumed)
    std::atomic<size_t> total{0};       // total graphs for the current size
    std::atomic<size_t> startCount{0};  // graphs already done before this run (resume)
    std::atomic<size_t> graphSize{0};   // number of vertices
    std::chrono::steady_clock::time_point startTime{};

    std::mutex mtx;
    std::string currentGraph;               // graph6 string of a graph currently being analyzed
    std::vector<std::string> currentMatrix; // adjacency-matrix rows of that graph, e.g. {"0 1 0", ...}

    // Records the graph currently being analyzed: its graph6 string plus the
    // adjacency-matrix rows rendered from its bitmask representation. Kept
    // dependency-free (just the raw bitmasks) so this header stays standalone.
    void setGraph(const std::string& g6, const std::vector<uint64_t>& adj, size_t n) {
        std::vector<std::string> rows;
        rows.reserve(n);
        for (size_t i = 0; i < n; ++i) {
            std::string r;
            r.reserve(2 * n);
            for (size_t j = 0; j < n; ++j) {
                if (j != 0) r += ' ';
                r += ((adj[i] >> j) & 1) ? '1' : '0';
            }
            rows.push_back(std::move(r));
        }
        std::lock_guard<std::mutex> lk(mtx);
        currentGraph = g6;
        currentMatrix = std::move(rows);
    }
    std::pair<std::string, std::vector<std::string>> snapshot() {
        std::lock_guard<std::mutex> lk(mtx);
        return {currentGraph, currentMatrix};
    }
};

// Renders a ProgressState to the terminal in its own thread using ANSI escapes,
// redrawing a few times per second. Header dependency-free so it can live
// alongside the header-only Graph class.
class Tui {
public:
    explicit Tui(ProgressState& state) : state_(state) {}

    ~Tui() { stop(); }

    void start() {
        running_.store(true);
        thread_ = std::thread([this] { loop(); });
    }

    void stop() {
        if (!running_.exchange(false)) return;
        if (thread_.joinable()) thread_.join();
        render();                  // leave a final, accurate frame on screen
        std::fputs("\033[?25h\n", stdout);  // restore cursor, drop below the box
        std::fflush(stdout);
    }

private:
    static constexpr int kInner = 60;  // inner width of the box, in chars
    static constexpr int kBar = 26;    // progress-bar cell count

    ProgressState& state_;
    std::thread thread_;
    std::atomic<bool> running_{false};

    void loop() {
        std::fputs("\033[?25l", stdout);  // hide cursor
        std::fputs("\033[2J", stdout);    // clear screen
        while (running_.load()) {
            render();
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }
    }

    static std::string formatDuration(double seconds) {
        if (seconds < 0 || !(seconds < 1e12)) return "--:--:--";
        long s = static_cast<long>(seconds);
        long h = s / 3600;
        long m = (s % 3600) / 60;
        long sec = s % 60;
        char buf[32];
        std::snprintf(buf, sizeof buf, "%02ld:%02ld:%02ld", h, m, sec);
        return buf;
    }

    // 12345678 -> "12,345,678"
    static std::string withCommas(size_t v) {
        std::string digits = std::to_string(v);
        std::string out;
        int count = 0;
        for (auto it = digits.rbegin(); it != digits.rend(); ++it) {
            if (count != 0 && count % 3 == 0) out.push_back(',');
            out.push_back(*it);
            ++count;
        }
        return std::string(out.rbegin(), out.rend());
    }

    // Display width of a UTF-8 string, counting code points (every byte that is
    // not a 0x80–0xBF continuation byte). The only non-ASCII glyphs we render
    // (— … █ ░) are all single-width, so code-point count equals column count.
    static size_t utf8Width(const std::string& s) {
        size_t w = 0;
        for (unsigned char c : s)
            if ((c & 0xC0) != 0x80) ++w;
        return w;
    }

    // Truncates to at most maxCp code points without splitting a multibyte char.
    static std::string utf8Truncate(const std::string& s, size_t maxCp) {
        size_t i = 0, cp = 0;
        while (i < s.size() && cp < maxCp) {
            unsigned char c = s[i];
            size_t len = (c >= 0xF0) ? 4 : (c >= 0xE0) ? 3 : (c >= 0xC0) ? 2 : 1;
            i += len;
            ++cp;
        }
        return s.substr(0, i);
    }

    // Prints one content row, padded/truncated to the box's inner width. Widths
    // are measured in display columns so multibyte glyphs stay aligned.
    static void row(const std::string& content) {
        std::string c = content;
        size_t w = utf8Width(c);
        if (static_cast<int>(w) > kInner) {
            c = utf8Truncate(c, kInner - 1) + "…";  // ellipsis on overflow
            w = kInner;
        }
        int pad = kInner - static_cast<int>(w);
        if (pad < 0) pad = 0;
        // "│ " ... " │" with \033[K to clear any leftover from a longer prior frame
        std::printf("│ %s%*s │\033[K\n", c.c_str(), pad, "");
    }

    static void rule(const char* left, const char* right) {
        std::string line;
        for (int i = 0; i < kInner + 2; ++i) line += "─";
        std::printf("%s%s%s\033[K\n", left, line.c_str(), right);
    }

    void render() {
        size_t processed = state_.processed.load();
        size_t total = state_.total.load();
        size_t n = state_.graphSize.load();

        double elapsed = std::chrono::duration<double>(
            std::chrono::steady_clock::now() - state_.startTime).count();

        // Rate measured over work done this run (exclude resumed-from count) so
        // ETA reflects the current session's throughput.
        size_t startCount = state_.startCount.load();
        size_t doneThisRun = processed >= startCount ? processed - startCount : 0;
        double rate = elapsed > 0.0 ? doneThisRun / elapsed : 0.0;

        size_t remaining = total > processed ? total - processed : 0;
        double eta = rate > 0.0 ? remaining / rate : -1.0;
        double pct = total > 0 ? 100.0 * processed / total : 0.0;

        auto [graph, matrix] = state_.snapshot();
        if (graph.empty()) graph = "(starting…)";

        // Progress bar
        int filled = total > 0
            ? static_cast<int>(kBar * (static_cast<double>(processed) / total))
            : 0;
        if (filled > kBar) filled = kBar;
        std::string bar;
        for (int i = 0; i < kBar; ++i) bar += (i < filled) ? "█" : "░";

        char buf[256];

        std::fputs("\033[H", stdout);  // cursor home — overwrite previous frame

        rule("╭", "╮");
        row("  Pinnacle Sets — Edge-Removal Analysis");
        rule("├", "┤");

        std::snprintf(buf, sizeof buf, "Graph size     : %zu vertices", n);
        row(buf);
        row("Current graph  : " + graph);
        row("");
        row("Adjacency matrix:");
        if (matrix.empty()) {
            row("  (starting…)");
        } else {
            for (const auto& mrow : matrix) row("  " + mrow);
        }
        row("");

        std::snprintf(buf, sizeof buf, "Progress       : %s %5.1f%%", bar.c_str(), pct);
        row(buf);
        std::snprintf(buf, sizeof buf, "Graphs         : %s / %s",
                      withCommas(processed).c_str(), withCommas(total).c_str());
        row(buf);
        std::snprintf(buf, sizeof buf, "Speed          : %s graphs/s",
                      withCommas(static_cast<size_t>(rate + 0.5)).c_str());
        row(buf);
        std::snprintf(buf, sizeof buf, "Elapsed        : %s", formatDuration(elapsed).c_str());
        row(buf);
        std::snprintf(buf, sizeof buf, "ETA            : %s", formatDuration(eta).c_str());
        row(buf);

        rule("╰", "╯");
        std::fputs("\033[J", stdout);  // clear anything below the box
        std::fflush(stdout);
    }
};
