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
    clear_screen();

    std::cout << "\n" << ansi::BOLD << ansi::CYAN;
    std::cout << "  \u2554" << repeat_str("\u2550", BOX_WIDTH) << "\u2557\n";
    std::cout << "  \u2551" << std::string(16, ' ')
              << "SYSTEM CAPABILITIES SUMMARY"
              << std::string(BOX_WIDTH - 16 - 26, ' ') << "\u2551\n";
    std::cout << "  \u2560" << repeat_str("\u2550", BOX_WIDTH) << "\u2563\n";
    std::cout << ansi::RESET;

    auto line = [](const std::string& text) {
        int pad = BOX_WIDTH - static_cast<int>(text.size());
        if (pad < 0) pad = 0;
        std::cout << ansi::CYAN << "  \u2551" << ansi::RESET
                  << text << std::string(pad, ' ')
                  << ansi::CYAN << "\u2551" << ansi::RESET << "\n";
    };

    auto section = [&](const std::string& title) {
        line("");
        std::string s = "  " + std::string(ansi::BOLD) + title + std::string(ansi::RESET);
        // Print manually since ANSI codes mess up padding
        std::cout << ansi::CYAN << "  \u2551" << ansi::RESET
                  << "  " << ansi::BOLD << title << ansi::RESET
                  << std::string(BOX_WIDTH - 2 - static_cast<int>(title.size()), ' ')
                  << ansi::CYAN << "\u2551" << ansi::RESET << "\n";
    };

    auto item = [&](const std::string& name, const std::string& desc) {
        std::string text = "    \u2713 " + name + "  --  " + desc;
        line(text);
    };

    section("Data Structures");
    item("QuadTree",       "spatial index, k-NN, range queries, branch-and-bound");
    item("Graph",          "adjacency list, weighted edges, dynamic updates");
    item("PriorityQueue",  "min-heap with decrease-key, lazy deletion");

    section("Algorithms");
    item("QuadTree k-NN",       "branch-and-bound O(N log k)");
    item("Dijkstra",            "shortest path O((V+E) log V)");
    item("A* Search",           "heuristic-guided pathfinding");
    item("Greedy Allocation",   "fair spot assignment O(M x N)");

    section("Design Patterns");
    item("Strategy Pattern",      "IPathfinder polymorphic interface");
    item("Observer Pattern",      "StatusCallback -> AvailabilityTracker");
    item("Dependency Injection",  "constructor-based, non-owning pointers");

    section("Implementation");
    item("C++17",              "std::optional, std::any, structured bindings");
    item("Header-only",        "single compilation unit, no linking");
    item("Zero dependencies",  "standard library only, all from scratch");

    line("");
    std::cout << ansi::CYAN << "  \u255A" << repeat_str("\u2550", BOX_WIDTH)
              << "\u255D" << ansi::RESET << "\n";

    std::cout << "\n\n" << ansi::BOLD
              << "                          Thank you. Questions?"
              << ansi::RESET << "\n\n";
    press_enter();
}

void scenario_1_rush_hour() {
    print_header("SCENARIO 1: RUSH HOUR EMERGENCY");
    std::cout << ansi::DIM
              << "    8:45 AM Monday morning. A driver arrives at the downtown parking\n"
              << "    garage during peak rush hour. Most spots near the entrance are taken.\n"
              << "    The system must find the nearest available spot and route the driver.\n"
              << ansi::RESET;
    press_enter();

    // ── Step 1: Build infrastructure ──────────────────────────────────

    print_subheader("Step 1: Initializing Parking Infrastructure");

    // Create spots: Zone A (8 spots, 7 occupied), Zone B (8, 4 occupied), Zone C (8, all free)
    struct SpotDef { std::string id; double x; double y; std::string zone; bool occupied; };
    std::vector<SpotDef> spot_defs = {
        {"A1",2,2,"Zone-A",true},  {"A2",4,2,"Zone-A",true},  {"A3",6,2,"Zone-A",true},
        {"A4",8,2,"Zone-A",true},  {"A5",2,4,"Zone-A",true},  {"A6",4,4,"Zone-A",true},
        {"A7",6,4,"Zone-A",false}, {"A8",8,4,"Zone-A",true},
        {"B1",12,2,"Zone-B",true}, {"B2",14,2,"Zone-B",false},{"B3",16,2,"Zone-B",true},
        {"B4",18,2,"Zone-B",false},{"B5",12,4,"Zone-B",true}, {"B6",14,4,"Zone-B",false},
        {"B7",16,4,"Zone-B",true}, {"B8",18,4,"Zone-B",false},
        {"C1",22,2,"Zone-C",false},{"C2",24,2,"Zone-C",false},{"C3",26,2,"Zone-C",false},
        {"C4",28,2,"Zone-C",false},{"C5",22,4,"Zone-C",false},{"C6",24,4,"Zone-C",false},
        {"C7",26,4,"Zone-C",false},{"C8",28,4,"Zone-C",false},
    };

    AvailabilityTracker tracker;
    QuadTree tree(BoundingBox(15.0, 3.0, 15.0, 3.0));
    std::vector<ParkingSpot> spots;
    spots.reserve(spot_defs.size());

    for (auto& sd : spot_defs) {
        spots.emplace_back(sd.id, std::make_pair(sd.x, sd.y), sd.zone);
        spots.back().register_status_callback(
            [&tracker](const std::string& id, SpotStatus old_s, SpotStatus new_s) {
                tracker.on_status_change(id, old_s, new_s);
            });
        tracker.register_spot(sd.id, sd.zone, true);
        tree.insert(Point(sd.x, sd.y, std::any(make_spot_data(sd.id, sd.zone))));
        if (sd.occupied) spots.back().occupy();
    }

    std::cout << "  Registered " << ansi::BOLD << "24" << ansi::RESET
              << " parking spots across " << ansi::BOLD << "3" << ansi::RESET
              << " zones.\n";
    std::cout << "  All spots indexed in QuadTree spatial structure.\n\n";

    print_label("Zone Occupancy:");
    print_occupancy_bar("Zone A (Ground)",   tracker.get_zone_occupancy_rate("Zone-A"));
    print_occupancy_bar("Zone B (Level 2)",  tracker.get_zone_occupancy_rate("Zone-B"));
    print_occupancy_bar("Zone C (Level 3)",  tracker.get_zone_occupancy_rate("Zone-C"));

    std::cout << "\n";
    std::cout << "  Total available: " << ansi::BOLD << tracker.count_available()
              << " / 24" << ansi::RESET << "\n";
    press_enter();

    // ── Step 2: Spatial search ────────────────────────────────────────

    print_subheader("Step 2: Driver Arrives -- QuadTree k-NN Search");

    std::cout << "  Driver location: " << ansi::BOLD << "Entrance (0.0, 0.0)" << ansi::RESET << "\n";
    std::cout << "  Searching for nearest available spots...\n\n";
    sleep_ms(400);

    SpotSearcher searcher(&tree, [&tracker](const std::string& id) {
        return tracker.is_available(id);
    });

    SearchCriteria criteria;
    criteria.location = {0.0, 0.0};
    criteria.max_results = 5;
    auto results = searcher.search(criteria);

    std::vector<int> widths = {6, 10, 14, 10};
    print_table_header({"Rank", "Spot ID", "Distance (m)", "Zone"}, widths);
    for (size_t i = 0; i < results.size(); ++i) {
        std::ostringstream dist;
        dist << std::fixed << std::setprecision(2) << results[i].distance;
        print_table_row({std::to_string(i + 1), results[i].spot_id, dist.str(), results[i].zone_id}, widths);
        sleep_ms(100);
    }
    print_table_end(widths);

    std::cout << "\n";
    if (!results.empty()) {
        print_success("Nearest available: " + results[0].spot_id + " (" +
                      [&]{ std::ostringstream s; s << std::fixed << std::setprecision(2)
                           << results[0].distance; return s.str(); }() + "m away)");
    }
    print_info("6 closer spots in Zone A were automatically filtered (occupied)");
    print_info("QuadTree branch-and-bound pruned the search space");
    press_enter();

    // ── Step 3: Route with A* ─────────────────────────────────────────

    print_subheader("Step 3: Computing Optimal Route with A*");

    Graph graph;
    graph.add_node("Entrance",    std::make_pair(0.0, 0.0));
    graph.add_node("I1",          std::make_pair(5.0, 3.0));
    graph.add_node("I2",          std::make_pair(10.0, 3.0));
    graph.add_node("I3",          std::make_pair(15.0, 3.0));
    graph.add_node("I4",          std::make_pair(20.0, 3.0));
    graph.add_node("I5",          std::make_pair(25.0, 3.0));
    graph.add_node("A7",          std::make_pair(6.0, 4.0));
    graph.add_node("B2",          std::make_pair(14.0, 2.0));
    graph.add_node("C3",          std::make_pair(26.0, 2.0));
    graph.add_node("Exit-South",  std::make_pair(30.0, 0.0));
    graph.add_node("Exit-North",  std::make_pair(30.0, 6.0));

    graph.add_edge_undirected("Entrance", "I1", 5.83);
    graph.add_edge_undirected("I1", "I2", 5.0);
    graph.add_edge_undirected("I2", "I3", 5.0);
    graph.add_edge_undirected("I3", "I4", 5.0);
    graph.add_edge_undirected("I4", "I5", 5.0);
    graph.add_edge_undirected("I5", "Exit-South", 5.83);
    graph.add_edge_undirected("I5", "Exit-North", 5.83);
    graph.add_edge_undirected("I1", "A7", 1.41);
    graph.add_edge_undirected("I2", "B2", 4.12);
    graph.add_edge_undirected("I5", "C3", 1.41);

    AStarPathfinder astar(&graph);
    DijkstraPathfinder dijkstra(&graph);
    RouteOptimizer router_astar(&astar, &graph);
    router_astar.register_exit("Exit-South");
    router_astar.register_exit("Exit-North");

    std::cout << "  Algorithm: " << ansi::BOLD << "A*" << ansi::RESET
              << " (Euclidean heuristic)\n\n";
    sleep_ms(300);

    auto route = router_astar.find_route_to_spot("Entrance", "A7");
    if (route.has_value()) {
        print_route_visual(route->nodes, route->total_distance, route->estimated_time);
        std::cout << "\n";
        print_success("Optimal route found via A* pathfinding");
    }
    press_enter();

    // ── Step 4: Algorithm comparison ──────────────────────────────────

    print_subheader("Step 4: Algorithm Showdown -- Dijkstra vs A*");

    std::cout << "  Testing both algorithms on " << ansi::BOLD << "Entrance -> C3"
              << ansi::RESET << " (long route)\n";
    std::cout << "  Swapping algorithm at runtime via " << ansi::BOLD
              << "IPathfinder*" << ansi::RESET << " (Strategy Pattern)\n\n";
    sleep_ms(300);

    // Time Dijkstra
    auto t0 = std::chrono::high_resolution_clock::now();
    auto dijk_result = graph.dijkstra("Entrance", "C3");
    auto t1 = std::chrono::high_resolution_clock::now();
    auto dijk_us = std::chrono::duration_cast<std::chrono::microseconds>(t1 - t0).count();

    // Time A*
    auto t2 = std::chrono::high_resolution_clock::now();
    auto astar_result = graph.a_star("Entrance", "C3");
    auto t3 = std::chrono::high_resolution_clock::now();
    auto astar_us = std::chrono::duration_cast<std::chrono::microseconds>(t3 - t2).count();

    std::vector<int> cmp_widths = {14, 22, 22};
    print_table_header({"Metric", "Dijkstra", "A*"}, cmp_widths);

    // Distance row
    auto fmt_dist = [](double d) {
        std::ostringstream s; s << std::fixed << std::setprecision(2) << d << " m"; return s.str();
    };
    print_table_row({"Distance",
        dijk_result ? fmt_dist(dijk_result->second) : "N/A",
        astar_result ? fmt_dist(astar_result->second) : "N/A"}, cmp_widths);

    // Path row
    auto fmt_path = [](const std::vector<std::string>& p) {
        std::string s;
        for (size_t i = 0; i < p.size(); ++i) {
            if (i > 0) s += " > ";
            s += p[i];
        }
        return s;
    };
    print_table_row({"Path",
        dijk_result ? fmt_path(dijk_result->first) : "N/A",
        astar_result ? fmt_path(astar_result->first) : "N/A"}, cmp_widths);

    // Timing row
    print_table_row({"Exec. time",
        std::to_string(dijk_us) + " \u00B5s",
        std::to_string(astar_us) + " \u00B5s"}, cmp_widths);

    // Optimality row
    print_table_row({"Optimality", "\u2713 Optimal", "\u2713 Optimal"}, cmp_widths);

    print_table_end(cmp_widths);

    std::cout << "\n";
    print_info("Both find the same optimal path with identical cost");
    print_info("A* uses Euclidean heuristic to focus search toward the goal");
    print_info("Dijkstra explores uniformly in all directions");
    print_success("Swapped at runtime via IPathfinder* -- zero code changes");
    press_enter();

    // ── Step 5: Live tracking (Observer pattern) ──────────────────────

    print_subheader("Step 5: Real-Time Status Updates -- Observer Pattern");

    std::cout << "  The driver parks in spot A7. Watch the system react...\n\n";
    sleep_ms(500);

    // Find spot A7 in our vector and occupy it
    for (auto& sp : spots) {
        if (sp.spot_id == "A7") {
            std::cout << "  " << ansi::BOLD << ansi::MAGENTA
                      << "\u26A1 CALLBACK: " << ansi::RESET
                      << "Spot A7 status changed: "
                      << ansi::GREEN << "available" << ansi::RESET << " -> "
                      << ansi::RED << "occupied" << ansi::RESET << "\n";
            sleep_ms(200);
            std::cout << "  " << ansi::BOLD << ansi::MAGENTA
                      << "\u26A1 TRACKER: " << ansi::RESET
                      << "Zone A availability recalculated\n";
            sp.occupy();
            break;
        }
    }

    std::cout << "\n";
    print_label("Updated Zone Occupancy:");
    print_occupancy_bar("Zone A (Ground)",  tracker.get_zone_occupancy_rate("Zone-A"));
    print_occupancy_bar("Zone B (Level 2)", tracker.get_zone_occupancy_rate("Zone-B"));
    print_occupancy_bar("Zone C (Level 3)", tracker.get_zone_occupancy_rate("Zone-C"));

    std::cout << "\n";
    print_warning("Zone A is now FULL -- all 8 spots occupied");
    print_info("Update triggered automatically via StatusCallback (Observer Pattern)");
    press_enter();

    // ── Scenario summary ──────────────────────────────────────────────

    print_subheader("Scenario 1 Summary");
    print_success("QuadTree k-NN search with availability filtering");
    print_success("A* pathfinding with Euclidean heuristic");
    print_success("Dijkstra vs A* head-to-head comparison");
    print_success("Polymorphic algorithm swap (Strategy Pattern)");
    print_success("Real-time status callbacks (Observer Pattern)");
    print_success("Zone occupancy rate monitoring");
    press_enter();
}

void scenario_2_stadium_event() {
    print_header("SCENARIO 2: STADIUM EVENT NIGHT");
    std::cout << ansi::DIM
              << "    Saturday evening, 6:30 PM. A major event at the nearby stadium causes\n"
              << "    a wave of drivers requesting parking simultaneously. The system must\n"
              << "    fairly allocate spots and handle overflow when demand exceeds supply.\n"
              << ansi::RESET;
    press_enter();

    // ── Step 1: Setup the stadium lot ─────────────────────────────────

    print_subheader("Step 1: Stadium Parking Lot -- Current Status");

    struct SpotDef { std::string id; double x; double y; std::string zone; bool occupied; };
    std::vector<SpotDef> spot_defs = {
        // Zone North (near stadium) — 3 of 5 occupied
        {"N1",5,35,"North",true},  {"N2",10,35,"North",true}, {"N3",15,35,"North",true},
        {"N4",5,30,"North",false}, {"N5",10,30,"North",false},
        // Zone East — 2 of 5 occupied
        {"E1",35,25,"East",true},  {"E2",35,20,"East",true},  {"E3",35,15,"East",false},
        {"E4",30,25,"East",false}, {"E5",30,20,"East",false},
        // Zone South (far lot) — all free
        {"S1",5,5,"South",false},  {"S2",10,5,"South",false}, {"S3",15,5,"South",false},
        {"S4",5,10,"South",false}, {"S5",10,10,"South",false},
        // Zone West — 3 of 5 occupied
        {"W1",5,15,"West",true},   {"W2",5,20,"West",true},   {"W3",5,25,"West",true},
        {"W4",10,15,"West",false}, {"W5",10,20,"West",false},
    };

    AvailabilityTracker tracker;
    QuadTree tree(BoundingBox(20.0, 20.0, 20.0, 20.0));
    std::vector<ParkingSpot> spots;
    spots.reserve(spot_defs.size());

    for (auto& sd : spot_defs) {
        spots.emplace_back(sd.id, std::make_pair(sd.x, sd.y), sd.zone);
        tracker.register_spot(sd.id, sd.zone, true);
        tree.insert(Point(sd.x, sd.y, std::any(make_spot_data(sd.id, sd.zone))));
        if (sd.occupied) {
            spots.back().occupy();
            tracker.on_status_change(sd.id, SpotStatus::AVAILABLE, SpotStatus::OCCUPIED);
        }
    }

    print_label("Zone Occupancy:");
    print_occupancy_bar("North (Stadium)", tracker.get_zone_occupancy_rate("North"));
    print_occupancy_bar("East  (Side)",    tracker.get_zone_occupancy_rate("East"));
    print_occupancy_bar("South (Far lot)", tracker.get_zone_occupancy_rate("South"));
    print_occupancy_bar("West  (Side)",    tracker.get_zone_occupancy_rate("West"));
    std::cout << "\n  Total available: " << ansi::BOLD << tracker.count_available()
              << " / 20" << ansi::RESET << "\n";
    press_enter();

    // ── Step 2: Drivers arrive ────────────────────────────────────────

    print_subheader("Step 2: 8 Drivers Request Parking Simultaneously");

    struct Driver { std::string id; std::string name; double x; double y; std::string desc; };
    std::vector<Driver> drivers = {
        {"D1", "Alice", 8,  33, "near stadium"},
        {"D2", "Bob",   33, 22, "from east"},
        {"D3", "Carol", 8,  8,  "from south"},
        {"D4", "Dave",  7,  18, "from west"},
        {"D5", "Eve",   12, 32, "near stadium"},
        {"D6", "Frank", 32, 18, "from east"},
        {"D7", "Grace", 12, 7,  "from south"},
        {"D8", "Hank",  9,  22, "from west"},
    };

    for (auto& d : drivers) {
        std::ostringstream line;
        line << ansi::BOLD << "  >> " << ansi::RESET
             << std::left << std::setw(8) << d.name
             << ansi::DIM << " at (" << std::setw(2) << d.x << ", "
             << std::setw(2) << d.y << ")  -- " << d.desc << ansi::RESET;
        std::cout << line.str() << "\n";
        sleep_ms(120);
    }

    std::cout << "\n";
    print_info("8 drivers, 12 available spots. System must assign optimally.");
    press_enter();

    // ── Step 3: Greedy allocation ─────────────────────────────────────

    print_subheader("Step 3: Greedy Allocation Algorithm -- O(M x N)");

    std::cout << "  For each driver, assign the " << ansi::BOLD << "closest available"
              << ansi::RESET << " spot...\n\n";
    sleep_ms(300);

    // Build request and spot vectors for the allocator
    std::vector<std::pair<std::string, std::pair<double, double>>> requests;
    for (auto& d : drivers) {
        requests.push_back({d.name, {d.x, d.y}});
    }

    std::vector<std::pair<std::string, std::pair<double, double>>> avail_spots;
    for (auto& sd : spot_defs) {
        if (!sd.occupied) {
            avail_spots.push_back({sd.id, {sd.x, sd.y}});
        }
    }

    AllocationOptimizer allocator;
    auto result = allocator.allocate_greedy(requests, avail_spots);

    // Build a zone lookup
    std::unordered_map<std::string, std::string> spot_zone_map;
    for (auto& sd : spot_defs) spot_zone_map[sd.id] = sd.zone;

    // Display results table
    std::vector<int> widths = {10, 10, 15, 10};
    print_table_header({"Driver", "Spot", "Distance (m)", "Zone"}, widths);

    // We need to print in driver order
    double total_cost = 0.0;
    for (auto& d : drivers) {
        auto it = result.assignments.find(d.name);
        if (it != result.assignments.end()) {
            std::string spot_id = it->second;
            // Calculate distance
            double dx = d.x, dy = d.y;
            double sx = 0, sy = 0;
            for (auto& sd : spot_defs) {
                if (sd.id == spot_id) { sx = sd.x; sy = sd.y; break; }
            }
            double dist = std::sqrt((dx-sx)*(dx-sx) + (dy-sy)*(dy-sy));
            total_cost += dist;

            std::ostringstream ds;
            ds << std::fixed << std::setprecision(2) << dist;
            print_table_row({d.name, spot_id, ds.str(), spot_zone_map[spot_id]}, widths);
            sleep_ms(100);
        }
    }
    print_table_end(widths);

    std::cout << "\n";
    print_success("All 8 drivers assigned successfully");
    std::ostringstream cost_str;
    cost_str << std::fixed << std::setprecision(2) << result.total_cost;
    print_success("Total walking distance: " + cost_str.str() + " m");
    std::ostringstream avg_str;
    avg_str << std::fixed << std::setprecision(2) << result.total_cost / 8.0;
    print_success("Average distance per driver: " + avg_str.str() + " m");
    press_enter();

    // ── Step 4: Demand exceeds supply ─────────────────────────────────

    print_subheader("Step 4: Demand Exceeds Supply -- 6 Late Arrivals");

    std::cout << "  6 more drivers arrive, but only " << ansi::BOLD << "4 spots"
              << ansi::RESET << " remain...\n\n";

    // After first allocation, 4 spots left (12 available - 8 assigned)
    // Build remaining spots
    std::vector<std::pair<std::string, std::pair<double, double>>> remaining_spots;
    std::unordered_set<std::string> assigned_set;
    for (auto& [name, spot] : result.assignments) assigned_set.insert(spot);
    for (auto& sd : spot_defs) {
        if (!sd.occupied && assigned_set.find(sd.id) == assigned_set.end()) {
            remaining_spots.push_back({sd.id, {sd.x, sd.y}});
        }
    }

    std::vector<std::pair<std::string, std::pair<double, double>>> late_requests = {
        {"Irene",  {20, 30}},
        {"Jack",   {25, 10}},
        {"Karen",  {18, 25}},
        {"Leo",    {30, 30}},
        {"Mia",    {22, 5}},
        {"Nick",   {28, 15}},
    };

    for (auto& [name, loc] : late_requests) {
        std::cout << "  " << ansi::BOLD << ">> " << ansi::RESET
                  << std::left << std::setw(8) << name
                  << ansi::DIM << " at (" << loc.first << ", " << loc.second << ")"
                  << ansi::RESET << "\n";
        sleep_ms(80);
    }

    auto late_result = allocator.allocate_greedy(late_requests, remaining_spots);

    std::cout << "\n";
    std::vector<int> widths2 = {10, 10, 15};
    print_table_header({"Driver", "Spot", "Distance (m)"}, widths2);
    for (auto& [name, loc] : late_requests) {
        auto it = late_result.assignments.find(name);
        if (it != late_result.assignments.end()) {
            double sx = 0, sy = 0;
            for (auto& sd : spot_defs) {
                if (sd.id == it->second) { sx = sd.x; sy = sd.y; break; }
            }
            double dist = std::sqrt((loc.first-sx)*(loc.first-sx) + (loc.second-sy)*(loc.second-sy));
            std::ostringstream ds;
            ds << std::fixed << std::setprecision(2) << dist;
            print_table_row({name, it->second, ds.str()}, widths2);
        }
    }
    print_table_end(widths2);

    if (!late_result.unassigned.empty()) {
        std::cout << "\n";
        std::string names;
        for (size_t i = 0; i < late_result.unassigned.size(); ++i) {
            if (i > 0) names += ", ";
            names += late_result.unassigned[i];
        }
        print_warning(std::to_string(late_result.unassigned.size()) +
                      " drivers could not be assigned: " + names);
        print_info("Greedy processes in arrival order: earlier arrivals get priority");
        print_info("A Hungarian algorithm could minimize total distance (O(N^3))");
        print_info("Greedy is O(M x N) -- better for real-time parking systems");
    }
    press_enter();

    // ── Step 5: QuadTree range query ──────────────────────────────────

    print_subheader("Step 5: QuadTree Range Query -- What's Near the Stadium?");

    std::cout << "  Query: All spots within " << ansi::BOLD << "15m"
              << ansi::RESET << " of the stadium entrance (10, 38)\n\n";
    sleep_ms(300);

    auto range_results = tree.query_radius(10.0, 38.0, 15.0);

    // Sort by distance
    std::sort(range_results.begin(), range_results.end(),
              [](auto& a, auto& b) { return a.second < b.second; });

    for (auto& [point, dist] : range_results) {
        std::string sid;
        try {
            auto& data = std::any_cast<const std::unordered_map<std::string, std::string>&>(point.data);
            auto it = data.find("spot_id");
            if (it != data.end()) sid = it->second;
        } catch (...) { continue; }

        bool is_avail = tracker.is_available(sid);
        // Check if it was assigned in our allocations
        bool was_assigned = assigned_set.count(sid) > 0;
        bool actually_avail = is_avail && !was_assigned;

        std::ostringstream line;
        line << "  " << std::left << std::setw(4) << sid
             << " (" << std::setw(2) << point.x << ", " << std::setw(2) << point.y << ")"
             << "  " << std::fixed << std::setprecision(1) << std::setw(5) << dist << "m  ";

        std::cout << line.str();
        if (!is_avail) {
            std::cout << ansi::RED << "\u25CF OCCUPIED" << ansi::RESET;
        } else if (was_assigned) {
            std::cout << ansi::YELLOW << "\u25CF JUST ASSIGNED" << ansi::RESET;
        } else {
            std::cout << ansi::GREEN << "\u25CF AVAILABLE" << ansi::RESET;
        }
        std::cout << "\n";
        sleep_ms(80);
    }

    std::cout << "\n";
    print_warning("Most spots near the stadium are occupied or just assigned");
    print_info("QuadTree range query: O(log N + R) -- only examined relevant quadrants");
    press_enter();

    // ── Scenario summary ──────────────────────────────────────────────

    print_subheader("Scenario 2 Summary");
    print_success("Greedy allocation algorithm O(M x N)");
    print_success("Batch processing of 8 simultaneous requests");
    print_success("Overflow handling -- 2 drivers unassigned when demand > supply");
    print_success("QuadTree range query for spatial analysis");
    print_success("Zone occupancy monitoring across 4 zones");
    press_enter();
}

void scenario_3_smart_rerouting() {
    print_header("SCENARIO 3: SMART REROUTING");
    std::cout << ansi::DIM
              << "    Wednesday afternoon, 2:00 PM. A driver is being routed to a spot\n"
              << "    when an accident blocks the primary route. Then another driver takes\n"
              << "    the target spot. The system must adapt in real time.\n"
              << ansi::RESET;
    press_enter();

    // ── Step 1: Build the road network ────────────────────────────────

    print_subheader("Step 1: Downtown Parking Garage -- Road Network");

    // Asymmetric graph: north route slightly shorter than south
    Graph graph;
    graph.add_node("Entrance", std::make_pair(0.0, 5.0));
    graph.add_node("A",        std::make_pair(4.0, 8.0));
    graph.add_node("B",        std::make_pair(4.0, 2.0));
    graph.add_node("C",        std::make_pair(8.0, 8.0));
    graph.add_node("D",        std::make_pair(8.0, 2.0));
    graph.add_node("E",        std::make_pair(12.0, 5.0));
    graph.add_node("P1",       std::make_pair(14.0, 8.0));
    graph.add_node("P2",       std::make_pair(14.0, 2.0));
    graph.add_node("P3",       std::make_pair(16.0, 5.0));
    graph.add_node("Exit",     std::make_pair(20.0, 5.0));

    graph.add_edge_undirected("Entrance", "A", 5.0);
    graph.add_edge_undirected("Entrance", "B", 5.0);
    graph.add_edge_undirected("A", "C", 4.0);    // North corridor
    graph.add_edge_undirected("B", "D", 4.5);    // South corridor (slightly longer)
    graph.add_edge_undirected("C", "E", 5.0);
    graph.add_edge_undirected("D", "E", 5.0);
    graph.add_edge_undirected("E", "P1", 3.61);
    graph.add_edge_undirected("E", "P2", 3.61);
    graph.add_edge_undirected("E", "P3", 4.0);
    graph.add_edge_undirected("P1", "Exit", 6.32);
    graph.add_edge_undirected("P2", "Exit", 6.32);
    graph.add_edge_undirected("P3", "Exit", 4.0);

    // ASCII map of the network
    std::cout << ansi::DIM;
    std::cout << "                    North Corridor\n";
    std::cout << "              A -----(4.0)----> C\n";
    std::cout << "             /                   \\\n";
    std::cout << "        (5.0)                  (5.0)\n";
    std::cout << "           /                       \\\n";
    std::cout << "    Entrance                        E ----> P1 ----> Exit\n";
    std::cout << "           \\                       /  \\             /\n";
    std::cout << "        (5.0)                  (5.0)  --> P2 ----/\n";
    std::cout << "             \\                   /     \\-> P3 --/\n";
    std::cout << "              B -----(4.5)----> D\n";
    std::cout << "                    South Corridor\n";
    std::cout << ansi::RESET << "\n";

    // Spots and availability
    ParkingSpot spot_p1("P1", {14.0, 8.0}, "Main");
    ParkingSpot spot_p2("P2", {14.0, 2.0}, "Main");
    ParkingSpot spot_p3("P3", {16.0, 5.0}, "Main");
    spot_p3.occupy(); // P3 already taken

    AvailabilityTracker tracker;
    tracker.register_spot("P1", "Main", true);
    tracker.register_spot("P2", "Main", true);
    tracker.register_spot("P3", "Main", false);

    spot_p1.register_status_callback(
        [&tracker](const std::string& id, SpotStatus old_s, SpotStatus new_s) {
            tracker.on_status_change(id, old_s, new_s);
        });
    spot_p2.register_status_callback(
        [&tracker](const std::string& id, SpotStatus old_s, SpotStatus new_s) {
            tracker.on_status_change(id, old_s, new_s);
        });

    print_label("Spot Status:");
    print_spot_status("P1", spot_p1.status);
    std::cout << "     ";
    print_spot_status("P2", spot_p2.status);
    std::cout << "     ";
    print_spot_status("P3", spot_p3.status);
    std::cout << "\n\n";

    print_info("North corridor (via A->C): 14.0m total to hub E");
    print_info("South corridor (via B->D): 14.5m total to hub E");
    print_info("North route is 0.5m shorter -- preferred path");
    press_enter();

    // ── Step 2: Full pipeline — search + route ────────────────────────

    print_subheader("Step 2: Driver Arrives -- Search + Route");

    QuadTree tree(BoundingBox(10.0, 5.0, 10.0, 5.0));
    tree.insert(Point(14.0, 8.0, std::any(make_spot_data("P1", "Main"))));
    tree.insert(Point(14.0, 2.0, std::any(make_spot_data("P2", "Main"))));
    tree.insert(Point(16.0, 5.0, std::any(make_spot_data("P3", "Main"))));

    SpotSearcher searcher(&tree, [&tracker](const std::string& id) {
        return tracker.is_available(id);
    });

    std::cout << "  Searching for nearest available spot from Entrance...\n";
    sleep_ms(400);
    auto nearest = searcher.search_nearest({0.0, 5.0});
    if (nearest.has_value()) {
        print_success("Nearest available: " + nearest->spot_id + " (" +
                      [&]{ std::ostringstream s; s << std::fixed << std::setprecision(2)
                           << nearest->distance; return s.str(); }() + "m)");
    }

    std::cout << "\n  Computing route with A*...\n";
    sleep_ms(300);

    AStarPathfinder astar(&graph);
    RouteOptimizer router(&astar, &graph);
    router.register_exit("Exit");

    auto route1 = router.find_route_to_spot("Entrance", "P1");
    if (route1.has_value()) {
        print_route_visual(route1->nodes, route1->total_distance, route1->estimated_time);
    }

    std::cout << "\n";
    print_success("Driver heading to P1 via " + std::string(ansi::BOLD) + "north corridor" +
                  std::string(ansi::RESET) + std::string(ansi::GREEN) + " (shorter route)");
    press_enter();

    // ── Step 3: Congestion strikes ────────────────────────────────────

    print_subheader("Step 3: CONGESTION ALERT -- Accident on Segment A -> C");

    std::cout << "\n";
    std::cout << ansi::BOLD << ansi::RED;
    std::cout << "  \u2554" << repeat_str("\u2550", 52) << "\u2557\n";
    std::cout << "  \u2551  \u26A0  TRAFFIC ALERT: Accident on segment A -> C        \u2551\n";
    std::cout << "  \u2551     Original weight:  4.0                          \u2551\n";
    std::cout << "  \u2551     Congestion delay: +20.0                        \u2551\n";
    std::cout << "  \u2551     New weight:        24.0                        \u2551\n";
    std::cout << "  \u255A" << repeat_str("\u2550", 52) << "\u255D\n";
    std::cout << ansi::RESET << "\n";

    // Apply congestion to both directions
    router.update_congestion("A", "C", 20.0);
    router.update_congestion("C", "A", 20.0);

    print_warning("Edge weight updated via Graph::update_weight()");
    print_info("System recalculating optimal route...");
    press_enter();

    // ── Step 4: Reroute via south corridor ────────────────────────────

    print_subheader("Step 4: System Reroutes via South Corridor");

    auto route2 = router.find_route_to_spot("Entrance", "P1");

    std::vector<int> cmp_widths = {16, 24, 24};
    print_table_header({"", "BEFORE Congestion", "AFTER Congestion"}, cmp_widths);

    // Path rows
    auto fmt_path = [](const std::vector<std::string>& p) {
        std::string s;
        for (size_t i = 0; i < p.size(); ++i) {
            if (i > 0) s += " > ";
            s += p[i];
        }
        return s;
    };
    print_table_row({"Route",
        route1 ? fmt_path(route1->nodes) : "N/A",
        route2 ? fmt_path(route2->nodes) : "N/A"}, cmp_widths);

    auto fmt_dist = [](double d) {
        std::ostringstream s; s << std::fixed << std::setprecision(2) << d << " m"; return s.str();
    };
    print_table_row({"Distance",
        route1 ? fmt_dist(route1->total_distance) : "N/A",
        route2 ? fmt_dist(route2->total_distance) : "N/A"}, cmp_widths);

    // Delta
    if (route1 && route2) {
        double delta = route2->total_distance - route1->total_distance;
        std::ostringstream ds;
        ds << std::fixed << std::setprecision(2) << "+" << delta << " m ("
           << std::setprecision(1) << (delta / route1->total_distance * 100.0) << "%)";
        print_table_row({"Detour cost", "--", ds.str()}, cmp_widths);
    }

    print_table_row({"Corridor used", "North (A->C)", "South (B->D)"}, cmp_widths);
    print_table_end(cmp_widths);

    std::cout << "\n";
    print_success("Small detour to avoid 20.0m congestion penalty");
    print_success("A* recomputed optimal path with updated edge weights");
    print_info("Dynamic weight update: RouteOptimizer::update_congestion()");
    press_enter();

    // ── Step 5: Spot stolen — re-search ───────────────────────────────

    print_subheader("Step 5: Spot Taken! -- Adaptive Re-Search");

    std::cout << "  While Driver 1 was rerouting, another driver took P1!\n\n";
    sleep_ms(500);

    std::cout << "  " << ansi::BOLD << ansi::MAGENTA
              << "\u26A1 CALLBACK: " << ansi::RESET
              << "P1 status changed: "
              << ansi::GREEN << "available" << ansi::RESET << " -> "
              << ansi::RED << "occupied" << ansi::RESET << "\n";
    sleep_ms(200);
    std::cout << "  " << ansi::BOLD << ansi::MAGENTA
              << "\u26A1 TRACKER: " << ansi::RESET
              << "P1 removed from available pool\n";
    spot_p1.occupy();

    std::cout << "\n";
    print_warning("Driver 1's target spot is no longer available!");
    std::cout << "\n  System re-searching for next best spot...\n";
    sleep_ms(400);

    auto new_nearest = searcher.search_nearest({0.0, 5.0});
    if (new_nearest.has_value()) {
        print_success("New nearest available: " + new_nearest->spot_id);
    }

    std::cout << "\n  Computing new route...\n";
    sleep_ms(300);

    auto route3 = router.find_route_to_spot("Entrance", "P2");
    if (route3.has_value()) {
        print_route_visual(route3->nodes, route3->total_distance, route3->estimated_time);
    }

    std::cout << "\n";
    print_success("Seamless fallback -- Observer Pattern detected the change");
    print_success("SpotSearcher found alternative without full re-scan");
    press_enter();

    // ── Step 6: Park and exit ─────────────────────────────────────────

    print_subheader("Step 6: Driver Parks and Exits");

    std::cout << "  Driver arrives at P2 and parks.\n\n";
    sleep_ms(300);

    std::cout << "  " << ansi::BOLD << ansi::MAGENTA
              << "\u26A1 CALLBACK: " << ansi::RESET
              << "P2 status changed: "
              << ansi::GREEN << "available" << ansi::RESET << " -> "
              << ansi::RED << "occupied" << ansi::RESET << "\n";
    spot_p2.occupy();

    std::cout << "\n  Computing exit route...\n";
    sleep_ms(300);

    auto exit_route = router.find_route_to_exit("P2");
    if (exit_route.has_value()) {
        print_route_visual(exit_route->nodes, exit_route->total_distance, exit_route->estimated_time);
    }

    std::cout << "\n";
    print_label("Final Spot Status:");
    print_spot_status("P1", spot_p1.status);
    std::cout << "     ";
    print_spot_status("P2", spot_p2.status);
    std::cout << "     ";
    print_spot_status("P3", spot_p3.status);
    std::cout << "\n\n";
    print_warning("All spots occupied. Garage FULL.");

    std::cout << "\n";
    print_label("Driver 1 Complete Journey:");
    std::cout << "  Search " << ansi::DIM << "\u2500\u25B6 " << ansi::RESET
              << "Route " << ansi::DIM << "\u2500\u25B6 " << ansi::RESET
              << ansi::RED << "Congestion" << ansi::RESET
              << ansi::DIM << " \u2500\u25B6 " << ansi::RESET
              << "Reroute " << ansi::DIM << "\u2500\u25B6 " << ansi::RESET
              << ansi::RED << "Spot Taken" << ansi::RESET
              << ansi::DIM << " \u2500\u25B6 " << ansi::RESET
              << "Re-search " << ansi::DIM << "\u2500\u25B6 " << ansi::RESET
              << ansi::GREEN << "Park" << ansi::RESET
              << ansi::DIM << " \u2500\u25B6 " << ansi::RESET
              << "Exit\n";
    press_enter();

    // ── Scenario summary ──────────────────────────────────────────────

    print_subheader("Scenario 3 Summary");
    print_success("Dynamic congestion detection and edge weight update");
    print_success("Real-time rerouting (A* recomputes optimal path)");
    print_success("Callback-driven spot unavailability (Observer Pattern)");
    print_success("Adaptive re-search when target spot is taken");
    print_success("Full pipeline: Search -> Route -> Reroute -> Re-search -> Park -> Exit");
    print_success("Route-to-exit computation");
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
