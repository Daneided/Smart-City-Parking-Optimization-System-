// Smart City Parking Optimization System — Presentation Demo
// Three real-world scenarios showcasing all algorithms and design patterns

#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <iomanip>
#include <sstream>
#include <chrono>
#include <thread>

#include "data_structures/quadtree.h"
#include "data_structures/graph.h"
#include "data_structures/priority_queue.h"
#include "models/parking_spot.h"
#include "models/availability_tracker.h"
#include "algorithms/search.h"
#include "algorithms/pathfinder.h"
#include "algorithms/optimization.h"

// ═══════════════════════════════════════════════════════════════
//  ANSI escape codes for terminal colors and formatting
// ═══════════════════════════════════════════════════════════════

namespace ansi {
    constexpr const char* RESET      = "\033[0m";
    constexpr const char* BOLD       = "\033[1m";
    constexpr const char* DIM        = "\033[2m";
    constexpr const char* ITALIC     = "\033[3m";
    constexpr const char* RED        = "\033[31m";
    constexpr const char* GREEN      = "\033[32m";
    constexpr const char* YELLOW     = "\033[33m";
    constexpr const char* BLUE       = "\033[34m";
    constexpr const char* MAGENTA    = "\033[35m";
    constexpr const char* CYAN       = "\033[36m";
    constexpr const char* WHITE      = "\033[37m";
    constexpr const char* BG_RED     = "\033[41m";
    constexpr const char* BG_GREEN   = "\033[42m";
    constexpr const char* BG_YELLOW  = "\033[43m";
    constexpr const char* BG_BLUE    = "\033[44m";
}

// ═══════════════════════════════════════════════════════════════
//  UI helper functions
// ═══════════════════════════════════════════════════════════════

static const int BOX_WIDTH = 70;

void clear_screen() {
    std::cout << "\033[2J\033[H";
}

void sleep_ms(int ms) {
    std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}

void press_enter() {
    std::cout << "\n" << ansi::DIM << ansi::ITALIC
              << "    Press Enter to continue..." << ansi::RESET;
    std::cin.get();
    std::cout << "\n";
}

// Repeat a UTF-8 string n times
static std::string repeat_str(const std::string& s, int n) {
    std::string result;
    for (int i = 0; i < n; ++i) result += s;
    return result;
}

// Double-line box for major headers
void print_header(const std::string& title) {
    int padding = (BOX_WIDTH - 2 - static_cast<int>(title.size())) / 2;
    if (padding < 0) padding = 0;

    std::cout << "\n" << ansi::BOLD << ansi::CYAN;
    std::cout << "  \u2554" << repeat_str("\u2550", BOX_WIDTH) << "\u2557\n";
    std::cout << "  \u2551" << std::string(padding, ' ') << title
              << std::string(BOX_WIDTH - padding - static_cast<int>(title.size()), ' ') << "\u2551\n";
    std::cout << "  \u255A" << repeat_str("\u2550", BOX_WIDTH) << "\u255D\n";
    std::cout << ansi::RESET << "\n";
}

// Single-line box for subsection headers
void print_subheader(const std::string& title) {
    std::cout << "\n" << ansi::BOLD << ansi::YELLOW;
    std::cout << "  \u250C" << repeat_str("\u2500", BOX_WIDTH) << "\u2510\n";
    std::cout << "  \u2502  " << title
              << std::string(BOX_WIDTH - 2 - static_cast<int>(title.size()), ' ') << "\u2502\n";
    std::cout << "  \u2514" << repeat_str("\u2500", BOX_WIDTH) << "\u2518\n";
    std::cout << ansi::RESET << "\n";
}

void print_success(const std::string& msg) {
    std::cout << "  " << ansi::BOLD << ansi::GREEN << "\u2713 " << ansi::RESET
              << ansi::GREEN << msg << ansi::RESET << "\n";
}

void print_warning(const std::string& msg) {
    std::cout << "  " << ansi::BOLD << ansi::YELLOW << "\u26A0 " << ansi::RESET
              << ansi::YELLOW << msg << ansi::RESET << "\n";
}

void print_info(const std::string& msg) {
    std::cout << "  " << ansi::BOLD << ansi::CYAN << "\u2139 " << ansi::RESET
              << ansi::CYAN << msg << ansi::RESET << "\n";
}

void print_label(const std::string& msg) {
    std::cout << "  " << ansi::BOLD << msg << ansi::RESET << "\n";
}

void print_occupancy_bar(const std::string& zone_name, double rate, int width = 40) {
    int filled = static_cast<int>(rate * width);
    if (filled > width) filled = width;
    int empty = width - filled;

    const char* color = ansi::GREEN;
    if (rate > 0.8) color = ansi::RED;
    else if (rate > 0.5) color = ansi::YELLOW;

    std::cout << "  " << std::left << std::setw(22) << zone_name;
    std::cout << color << "[";
    for (int i = 0; i < filled; ++i) std::cout << "\u2588";
    for (int i = 0; i < empty; ++i) std::cout << "\u2591";
    std::cout << "]" << ansi::RESET;

    std::ostringstream pct;
    pct << std::fixed << std::setprecision(1) << (rate * 100.0) << "%";
    std::cout << "  " << std::setw(6) << std::right << pct.str();

    if (rate >= 1.0) std::cout << "  " << ansi::RED << ansi::BOLD << "FULL" << ansi::RESET;
    else if (rate > 0.8) std::cout << "  " << ansi::YELLOW << "HIGH" << ansi::RESET;
    else if (rate < 0.01) std::cout << "  " << ansi::GREEN << "EMPTY" << ansi::RESET;
    std::cout << "\n";
}

void print_spot_status(const std::string& id, SpotStatus status) {
    const char* color = ansi::WHITE;
    std::string label = spot_status_to_string(status);
    switch (status) {
        case SpotStatus::AVAILABLE:   color = ansi::GREEN;   break;
        case SpotStatus::OCCUPIED:    color = ansi::RED;     break;
        case SpotStatus::RESERVED:    color = ansi::YELLOW;  break;
        case SpotStatus::MAINTENANCE: color = ansi::MAGENTA; break;
        default: break;
    }
    std::cout << "  " << color << "\u25CF " << ansi::RESET
              << std::left << std::setw(6) << id << " "
              << color << label << ansi::RESET;
}

void print_route_visual(const std::vector<std::string>& nodes, double distance, double time) {
    std::cout << "  " << ansi::BOLD << "Route: " << ansi::RESET;
    for (size_t i = 0; i < nodes.size(); ++i) {
        if (i > 0) std::cout << ansi::DIM << " \u2500\u2500\u25B6 " << ansi::RESET;
        std::cout << ansi::BOLD << ansi::WHITE << nodes[i] << ansi::RESET;
    }
    std::cout << "\n";
    std::ostringstream info;
    info << std::fixed << std::setprecision(2);
    std::cout << "  Distance: " << ansi::BOLD << distance << " m" << ansi::RESET
              << "  |  Time: " << ansi::BOLD << time << " s" << ansi::RESET << "\n";
}

// Print a table separator line using UTF-8 box-drawing strings
void print_table_sep(const std::vector<int>& widths,
                     const std::string& left, const std::string& mid,
                     const std::string& right, const std::string& fill) {
    std::cout << "  " << left;
    for (size_t i = 0; i < widths.size(); ++i) {
        std::cout << repeat_str(fill, widths[i] + 2);
        if (i + 1 < widths.size()) std::cout << mid;
    }
    std::cout << right << "\n";
}

void print_table_header(const std::vector<std::string>& headers, const std::vector<int>& widths) {
    print_table_sep(widths, "\u250C", "\u252C", "\u2510", "\u2500");
    std::cout << "  \u2502";
    for (size_t i = 0; i < headers.size(); ++i) {
        std::cout << " " << ansi::BOLD << std::left << std::setw(widths[i])
                  << headers[i] << ansi::RESET << " \u2502";
    }
    std::cout << "\n";
    print_table_sep(widths, "\u251C", "\u253C", "\u2524", "\u2500");
}

void print_table_row(const std::vector<std::string>& cells, const std::vector<int>& widths,
                     const char* color = nullptr) {
    std::cout << "  \u2502";
    for (size_t i = 0; i < cells.size(); ++i) {
        std::cout << " ";
        if (color) std::cout << color;
        std::cout << std::left << std::setw(widths[i]) << cells[i];
        if (color) std::cout << ansi::RESET;
        std::cout << " \u2502";
    }
    std::cout << "\n";
}

void print_table_end(const std::vector<int>& widths) {
    print_table_sep(widths, "\u2514", "\u2534", "\u2518", "\u2500");
}

// Helper to build QuadTree point data
std::unordered_map<std::string, std::string> make_spot_data(
        const std::string& spot_id, const std::string& zone_id,
        const std::string& spot_type = "standard") {
    return {{"spot_id", spot_id}, {"zone_id", zone_id}, {"spot_type", spot_type}};
}

// ═══════════════════════════════════════════════════════════════
//  Welcome screen and menu
// ═══════════════════════════════════════════════════════════════

void show_welcome() {
    clear_screen();
    std::cout << "\n\n";
    print_header("SMART CITY PARKING OPTIMIZATION SYSTEM");

    std::cout << ansi::DIM;
    std::cout << "    A system that finds, allocates, and routes drivers to parking spots.\n";
    std::cout << "    All algorithms implemented from scratch in C++17. No external dependencies.\n";
    std::cout << ansi::RESET << "\n";

    std::cout << "  " << ansi::BOLD << "Algorithms:  " << ansi::RESET
              << "QuadTree k-NN  |  Dijkstra  |  A*  |  Greedy Allocation\n";
    std::cout << "  " << ansi::BOLD << "Patterns:    " << ansi::RESET
              << "Strategy  |  Observer  |  Dependency Injection\n";
    std::cout << "  " << ansi::BOLD << "Language:    " << ansi::RESET
              << "C++17  |  Standard Library Only  |  Header-Only\n";
}

int show_menu() {
    std::cout << "\n";
    std::cout << "  " << ansi::BOLD << ansi::CYAN
              << "\u250C\u2500\u2500\u2500\u2500\u2500\u2500\u2500\u2500\u2500\u2500"
              << "\u2500\u2500\u2500\u2500\u2500\u2500\u2500\u2500\u2500\u2500"
              << "\u2500\u2500\u2500\u2500\u2500\u2500\u2500\u2500\u2500\u2500"
              << "\u2500\u2500\u2500\u2500\u2500\u2500\u2500\u2500\u2500\u2500"
              << "\u2500\u2500\u2500\u2500\u2500\u2500\u2500\u2500\u2500\u2500\u2510" << ansi::RESET << "\n";
    std::cout << "  " << ansi::CYAN << "\u2502" << ansi::RESET
              << ansi::BOLD << "  [1] " << ansi::RESET << "Rush Hour Emergency  \u2014  Search & Routing     "
              << ansi::CYAN << "\u2502" << ansi::RESET << "\n";
    std::cout << "  " << ansi::CYAN << "\u2502" << ansi::RESET
              << ansi::BOLD << "  [2] " << ansi::RESET << "Stadium Event Night  \u2014  Mass Allocation     "
              << ansi::CYAN << "\u2502" << ansi::RESET << "\n";
    std::cout << "  " << ansi::CYAN << "\u2502" << ansi::RESET
              << ansi::BOLD << "  [3] " << ansi::RESET << "Smart Rerouting      \u2014  Congestion Control  "
              << ansi::CYAN << "\u2502" << ansi::RESET << "\n";
    std::cout << "  " << ansi::CYAN << "\u2502" << ansi::RESET
              << "                                                  "
              << ansi::CYAN << "\u2502" << ansi::RESET << "\n";
    std::cout << "  " << ansi::CYAN << "\u2502" << ansi::RESET
              << ansi::BOLD << "  [A] " << ansi::RESET << "Run All Scenarios                             "
              << ansi::CYAN << "\u2502" << ansi::RESET << "\n";
    std::cout << "  " << ansi::CYAN << "\u2502" << ansi::RESET
              << ansi::BOLD << "  [Q] " << ansi::RESET << "Quit                                          "
              << ansi::CYAN << "\u2502" << ansi::RESET << "\n";
    std::cout << "  " << ansi::BOLD << ansi::CYAN
              << "\u2514\u2500\u2500\u2500\u2500\u2500\u2500\u2500\u2500\u2500\u2500"
              << "\u2500\u2500\u2500\u2500\u2500\u2500\u2500\u2500\u2500\u2500"
              << "\u2500\u2500\u2500\u2500\u2500\u2500\u2500\u2500\u2500\u2500"
              << "\u2500\u2500\u2500\u2500\u2500\u2500\u2500\u2500\u2500\u2500"
              << "\u2500\u2500\u2500\u2500\u2500\u2500\u2500\u2500\u2500\u2500\u2518" << ansi::RESET << "\n";

    std::cout << "\n  " << ansi::BOLD << "Select: " << ansi::RESET;
    std::string input;
    std::getline(std::cin, input);
    if (input.empty()) return -1;
    char c = input[0];
    if (c == '1') return 1;
    if (c == '2') return 2;
    if (c == '3') return 3;
    if (c == 'a' || c == 'A') return 4;
    if (c == 'q' || c == 'Q') return 0;
    return -1;
}

void show_finale() {
    // Placeholder — will be implemented in a later commit
}

void scenario_1_rush_hour() {
    // Placeholder
    print_header("SCENARIO 1: RUSH HOUR EMERGENCY");
    std::cout << "  Coming soon...\n";
    press_enter();
}

void scenario_2_stadium_event() {
    // Placeholder
    print_header("SCENARIO 2: STADIUM EVENT NIGHT");
    std::cout << "  Coming soon...\n";
    press_enter();
}

void scenario_3_smart_rerouting() {
    // Placeholder
    print_header("SCENARIO 3: SMART REROUTING");
    std::cout << "  Coming soon...\n";
    press_enter();
}

int main() {
    show_welcome();

    while (true) {
        int choice = show_menu();
        switch (choice) {
            case 1:
                clear_screen();
                scenario_1_rush_hour();
                show_welcome();
                break;
            case 2:
                clear_screen();
                scenario_2_stadium_event();
                show_welcome();
                break;
            case 3:
                clear_screen();
                scenario_3_smart_rerouting();
                show_welcome();
                break;
            case 4:
                clear_screen();
                scenario_1_rush_hour();
                scenario_2_stadium_event();
                scenario_3_smart_rerouting();
                show_finale();
                show_welcome();
                break;
            case 0:
                std::cout << "\n  " << ansi::DIM << "Goodbye.\n" << ansi::RESET << "\n";
                return 0;
            default:
                break;
        }
    }
}
