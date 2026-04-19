# Smart City Parking Optimization System

## Build & Run

```bash
# Minimal integration demo
g++ -std=c++17 -I src src/main.cpp -o SmartCityParking
./SmartCityParking

# Interactive presentation demo (three scenarios)
g++ -std=c++17 -I src src/demo.cpp -o SmartCityDemo
./SmartCityDemo

# Test suite
g++ -std=c++17 -I src -I tests tests/run_tests.cpp tests/test_*.cpp -o run_tests
./run_tests
```

Requires a C++17 compiler (g++ 7+ or clang++ 5+). No external dependencies — C++ standard library only.

## What it does

Finds, allocates, and routes drivers to parking spots. Given a driver's location, it locates the nearest available spot, returns the shortest route, and handles fair assignment when multiple drivers compete.

Four subsystems:

1. **Spot search** — QuadTree spatial index with branch-and-bound k-NN pruning; only available spots are returned and the search radius doubles adaptively until a match is found.
2. **Pathfinding** — A\* (Euclidean heuristic) and Dijkstra over a weighted directed graph, swappable at runtime via the `IPathfinder` strategy interface.
3. **Allocation** — Greedy O(M×N) assignment: each driver gets the closest remaining spot in arrival order.
4. **Availability tracking** — Spots fire `StatusCallback` on state change; `AvailabilityTracker` maintains zone-level occupancy via observer callbacks.

## Project layout

```
src/
├── main.cpp                        # Minimal demo
├── demo.cpp                        # Interactive three-scenario demo
├── models/
│   ├── common_types.h              # Coordinate, SpotData, AllocEntry
│   ├── parking_spot.h              # Spot model with status callbacks
│   └── availability_tracker.h      # Zone-level availability tracking
├── data_structures/
│   ├── quadtree.h                  # 2D spatial index (k-NN, range query)
│   └── graph.h                     # Weighted directed graph + Dijkstra + A*
└── algorithms/
    ├── search.h                    # SpotSearcher
    ├── pathfinder.h                # IPathfinder + Dijkstra/A* strategies
    └── optimization.h              # RouteOptimizer + AllocationOptimizer

tests/                              # Custom header-only test framework
```

## Design

Constructor-based dependency injection with non-owning pointers. `SpotSearcher` takes a `QuadTree*` + availability checker; `RouteOptimizer` takes an `IPathfinder*` + `Graph*`; `AllocationOptimizer` is stateless. `DijkstraPathfinder` and `AStarPathfinder` both implement `IPathfinder` — strategy pattern. `ParkingSpot` fires `StatusCallback`, `AvailabilityTracker` listens — observer pattern.

## Complexity

| Operation | Time | Space |
|---|---|---|
| QuadTree insert | O(log N) avg | O(N) |
| QuadTree k-NN | O(N log k) worst | O(k) |
| QuadTree range query | O(log N + R) | O(R) |
| Dijkstra / A\* | O((V + E) log V) | O(V) |
| Greedy allocation | O(M × N) | O(M + N) |
| Availability update | O(1) | O(S) |
