# Smart City Parking Optimization System

A system that finds, allocates, and routes drivers to parking spots. Given a driver's location, it locates the nearest available spot, provides the shortest route to it, and handles fair assignment when multiple drivers compete for the same spots.

## How It Works

1. **Spot Search** — A QuadTree spatial index narrows down nearby spots without scanning every one. Only available spots are returned.
2. **Pathfinding** — The parking area is modeled as a graph. A* and Dijkstra find the shortest route from the driver to the assigned spot (and back to the exit).
3. **Allocation** — A greedy algorithm assigns each driver to the closest open spot, processing requests as they arrive.
4. **Availability Tracking** — Every spot reports its status (open, occupied, reserved, maintenance) and zone-level occupancy updates automatically.

## Project Structure

```
src/
├── main.cpp                        # Integration demo
├── models/
│   ├── parking_spot.h              # Spot model with status callbacks
│   └── availability_tracker.h      # Zone-level availability tracking
├── data_structures/
│   ├── quadtree.h                  # Spatial index for spot lookup
│   ├── graph.h                     # Road network (adjacency list)
│   └── priority_queue.h            # Min-heap for pathfinding
└── algorithms/
    ├── search.h                    # QuadTree k-NN spot search
    ├── pathfinder.h                # A* and Dijkstra (polymorphic)
    └── optimization.h              # Route optimizer + greedy allocation
```

Header-only C++17. No external dependencies — everything is built from scratch using the standard library.

## Build & Run

```bash
g++ -std=c++17 -I src src/main.cpp -o SmartCityParking
./SmartCityParking
```

Requires a C++17 compiler.
