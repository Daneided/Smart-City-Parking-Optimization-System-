# Smart City Parking Optimization System

A system that finds, allocates, and routes drivers to parking spots. Given a driver's location, it locates the nearest available spot, provides the shortest route to it, and handles fair assignment when multiple drivers compete for the same spots.

Everything is implemented from scratch in C++17 using only the standard library — no external dependencies.

## How It Works

The system has four subsystems that work together:

1. **Spot Search** — A QuadTree spatial index narrows down nearby spots without scanning every one. Uses a branch-and-bound k-nearest-neighbor algorithm with max-heap pruning to find the closest available spots efficiently. Only available spots are returned; the searcher adaptively doubles its search radius until a valid spot is found.

2. **Pathfinding** — The parking area is modeled as a weighted directed graph. Two pathfinding algorithms are available through a polymorphic interface (`IPathfinder`): A\* uses Euclidean distance as an admissible heuristic to focus the search toward the goal, while Dijkstra explores all directions uniformly as a guaranteed baseline. Both produce optimal shortest paths, and the algorithm can be swapped at runtime.

3. **Allocation** — A greedy algorithm assigns each driver to the closest open spot, processing requests in arrival order. Each assignment removes the spot from the available pool, ensuring no double-booking. The algorithm runs in O(M x N) time where M is the number of requests and N is the number of available spots.

4. **Availability Tracking** — Every spot reports its status (available, occupied, reserved, maintenance) and triggers callbacks on status change. The `AvailabilityTracker` maintains zone-level occupancy rates that update automatically when any spot's status changes.

## Algorithm Complexity

| Algorithm | Time Complexity | Space Complexity | Notes |
|---|---|---|---|
| QuadTree insert | O(log N) avg | O(N) | Recursive subdivision with capacity limit |
| QuadTree k-NN | O(N log k) worst | O(k) | Branch-and-bound with max-heap pruning |
| QuadTree range query | O(log N + R) | O(R) | R = number of results |
| Dijkstra shortest path | O((V + E) log V) | O(V) | Standard min-heap implementation |
| A\* search | O((V + E) log V) | O(V) | Euclidean heuristic; falls back to Dijkstra if no positions |
| Priority Queue push/pop | O(log N) | O(N) | Custom min-heap with lazy deletion |
| Priority Queue decrease-key | O(log N) amortized | O(1) | Re-insert with lower priority, mark old entry removed |
| Greedy allocation | O(M x N) | O(M + N) | M requests, N spots; Euclidean distance |
| Availability tracking | O(1) lookup/update | O(S) | S = total registered spots |

## Design Decisions

**Why QuadTree for spatial search?** Parking spots are distributed in 2D space. A QuadTree partitions this space recursively, pruning entire quadrants during search. For k-nearest-neighbor queries, the branch-and-bound approach avoids examining every point — it skips entire subtrees whose minimum bounding-box distance exceeds the k-th best candidate found so far. This is significantly faster than a linear scan for large parking structures.

**Why A\* over Dijkstra alone?** Dijkstra explores outward uniformly in all directions. A\* adds an admissible heuristic (Euclidean distance to the goal) that biases the search toward the target, reducing the number of nodes visited. Both are implemented and swappable at runtime via the `IPathfinder` interface — Dijkstra serves as the reliable baseline, A\* as the optimized path.

**Why a custom Priority Queue?** The standard `std::priority_queue` does not support decrease-key, which is needed for efficient A\* and Dijkstra implementations. The custom priority queue uses lazy deletion (marking removed entries instead of extracting them) and a counter for deterministic tie-breaking.

**Why greedy allocation?** Optimal assignment (e.g., Hungarian algorithm) is O(N^3) and unnecessary for real-time parking where requests arrive sequentially. The greedy approach assigns each driver to the closest available spot in arrival order — simple, fast, and produces good results in practice for the typical case where spots outnumber concurrent requests.

**Why header-only C++17?** Header-only design eliminates linking complexity — the entire project compiles with a single `g++` invocation. C++17 features (`std::optional` for nullable returns, `std::any` for generic QuadTree point data, structured bindings, `std::function` for callbacks) reduce boilerplate and improve safety.

**Why no external libraries?** All algorithms are implemented from scratch to demonstrate understanding of the underlying data structures and their trade-offs. No parts of the solution are trivialized by library calls.

## Project Structure

```
src/
├── main.cpp                        # Integration demo
├── models/
│   ├── parking_spot.h              # Spot model with status callbacks
│   └── availability_tracker.h      # Zone-level availability tracking
├── data_structures/
│   ├── quadtree.h                  # Spatial index for spot lookup (branch-and-bound k-NN)
│   ├── graph.h                     # Road network (adjacency list, Dijkstra, A*)
│   └── priority_queue.h            # Min-heap with decrease-key for pathfinding
└── algorithms/
    ├── search.h                    # QuadTree k-NN spot search with adaptive doubling
    ├── pathfinder.h                # IPathfinder interface + Dijkstra/A* implementations
    └── optimization.h              # Route optimizer + greedy allocation

tests/
├── test_framework.h                # Lightweight custom test framework (no dependencies)
├── test_priority_queue.cpp         # 17 tests — ordering, decrease-key, lazy deletion
├── test_quadtree.cpp               # 48 tests — insert, remove, range query, k-NN, merging
├── test_graph.cpp                  # 32 tests — Dijkstra, A*, edge cases, algorithm comparison
├── test_search.cpp                 # 13 tests — SpotSearcher nearest, filtering, availability
└── test_optimization.cpp           # 16 tests — AllocationOptimizer, RouteOptimizer, congestion
```

## Architecture

The system uses constructor-based dependency injection with non-owning pointers. Classes are composed, not tightly coupled:

- `SpotSearcher` takes a `QuadTree*` and a `std::function<bool(const std::string&)>` availability checker
- `RouteOptimizer` takes an `IPathfinder*` and a `Graph*`
- `IPathfinder` is an abstract base class; `DijkstraPathfinder` and `AStarPathfinder` inherit from it (strategy pattern)
- `ParkingSpot` triggers `StatusCallback` on status change; `AvailabilityTracker` listens via these callbacks (observer pattern)
- `AllocationOptimizer` is stateless — it takes requests and spots as parameters

This design allows swapping algorithms at runtime (e.g., switching from A\* to Dijkstra) and testing components in isolation.

## Build & Run

```bash
g++ -std=c++17 -I src src/main.cpp -o SmartCityParking
./SmartCityParking
```

Requires a C++17 compiler (g++ 7+ or clang++ 5+).

### Expected Output

```
=== Smart City Parking Optimization System ===

Available spots: 3
After occupying S2: 2 available
Zone-A occupancy rate: 0.5

Search results (from origin):
  S1 dist=2.23607 zone=zone-A
  S3 dist=12.0416 zone=zone-B
Nearest available: S1

Using pathfinder: A*
Route entrance -> S1: entrance -> intersect1 -> S1 (dist=3.83, time=0.766s)
Route S3 -> exit: S3 -> intersect2 -> exit (dist=12.07)

Using pathfinder: Dijkstra
Route entrance -> S1: entrance -> intersect1 -> S1 (dist=3.83, time=0.766s)

Greedy allocation:
  req1 -> S1
  req2 -> S3
Total cost: 3.65028

=== All systems operational ===
```

### Running Tests

```bash
# Compile and run all tests
g++ -std=c++17 -I src -I tests tests/test_priority_queue.cpp tests/test_quadtree.cpp tests/test_graph.cpp tests/test_search.cpp tests/test_optimization.cpp -x c++ - -o test_runner <<< '#include "test_framework.h"
int main() { return run_all_tests(); }'
./test_runner
```

## Usage Examples

### Finding the nearest available spot

```cpp
QuadTree tree(BoundingBox(0, 0, 100, 100));
tree.insert(Point(10, 10, spot_data));  // spot_data is unordered_map with spot_id, zone_id, spot_type

AvailabilityTracker tracker;
tracker.register_spot("S1", "zone-A", true);

SpotSearcher searcher(&tree, [&tracker](const std::string& id) {
    return tracker.is_available(id);
});

auto nearest = searcher.search_nearest({0.0, 0.0});
if (nearest.has_value()) {
    std::cout << "Nearest: " << nearest->spot_id
              << " at distance " << nearest->distance << std::endl;
}
```

### Routing a driver to a spot

```cpp
Graph graph;
graph.add_node("entrance", {{0.0, 0.0}});
graph.add_node("S1", {{5.0, 5.0}});
graph.add_edge_undirected("entrance", "S1", 7.07);

AStarPathfinder pathfinder(&graph);
RouteOptimizer router(&pathfinder, &graph);
router.register_exit("exit");

auto route = router.find_route_to_spot("entrance", "S1");
if (route.has_value()) {
    std::cout << "Distance: " << route->total_distance
              << " Time: " << route->estimated_time << "s" << std::endl;
}
```

### Allocating spots to multiple drivers

```cpp
AllocationOptimizer allocator;
std::vector<std::pair<std::string, std::pair<double, double>>> requests = {
    {"driver1", {0.0, 0.0}},
    {"driver2", {7.0, 8.0}},
};
std::vector<std::pair<std::string, std::pair<double, double>>> spots = {
    {"S1", {1.0, 2.0}},
    {"S2", {8.0, 9.0}},
};

auto result = allocator.allocate_greedy(requests, spots);
for (const auto& [driver, spot] : result.assignments) {
    std::cout << driver << " -> " << spot << std::endl;
}
```

## Known Limitations

- **Static graph weights**: Edge weights are set at initialization. The `update_congestion()` method allows manual adjustment, but there is no real-time traffic feed.
- **Greedy allocation is not globally optimal**: Processing requests in arrival order means earlier drivers get better spots. A Hungarian algorithm variant could minimize total distance across all assignments, but at O(N^3) cost.
- **QuadTree performance with clustered points**: If many points share nearly identical coordinates, the tree subdivides deeply. The merge heuristic mitigates this on removal but doesn't prevent deep insertion.
- **No concurrent access**: The system is single-threaded. Concurrent drivers would require synchronization around the QuadTree and AvailabilityTracker.
- **2D Euclidean distances only**: The A\* heuristic and allocation use straight-line distance. In a real parking structure with floors or one-way lanes, Manhattan distance or actual driving distance would be more appropriate.
