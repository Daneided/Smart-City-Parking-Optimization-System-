# Optimization algorithms for route planning and spot allocation

from typing import List, Tuple, Optional, Dict, Set
from dataclasses import dataclass


@dataclass
class Route:
    """Path through parking area with distance and time estimates"""
    nodes: List[str]
    total_distance: float
    estimated_time: float


@dataclass
class AllocationResult:
    """Result of batch spot allocation"""
    assignments: Dict[str, str]  # request_id -> spot_id
    total_cost: float
    unassigned: List[str]


class RouteOptimizer:
    """
    Finds optimal routes within parking areas.
    Uses Dijkstra's and A* for pathfinding.

    Features:
    - Entrance to spot routing
    - Spot to exit routing
    - Congestion-aware weight updates
    """

    # Assumed travel speed in a parking lot (distance units per second)
    PARKING_SPEED = 5.0

    def __init__(self, parking_graph=None):
        self._graph = parking_graph
        self._exits: Set[str] = set()

    def register_exit(self, exit_id: str) -> None:
        """Register a node as an exit so find_route_to_exit can search all exits."""
        self._exits.add(exit_id)

    def find_route_to_spot(self, entrance_id: str, spot_id: str) -> Optional[Route]:
        """A* search with euclidean distance heuristic"""
        if self._graph is None:
            return None
        result = self._graph.a_star(entrance_id, spot_id)
        if result is None:
            return None
        path, distance = result
        return Route(
            nodes=path,
            total_distance=distance,
            estimated_time=distance / self.PARKING_SPEED,
        )

    def find_route_to_exit(self, spot_id: str, preferred_exit: Optional[str] = None) -> Optional[Route]:
        """Find best route to exit, optionally preferring a specific one."""
        if self._graph is None:
            return None

        # If a specific exit is requested, route directly to it
        if preferred_exit is not None:
            result = self._graph.a_star(spot_id, preferred_exit)
            if result is None:
                return None
            path, distance = result
            return Route(
                nodes=path,
                total_distance=distance,
                estimated_time=distance / self.PARKING_SPEED,
            )

        # Otherwise try all registered exits and pick the shortest
        best: Optional[Route] = None
        for exit_id in self._exits:
            result = self._graph.a_star(spot_id, exit_id)
            if result is None:
                continue
            path, distance = result
            if best is None or distance < best.total_distance:
                best = Route(
                    nodes=path,
                    total_distance=distance,
                    estimated_time=distance / self.PARKING_SPEED,
                )
        return best

    def update_congestion(self, edge_id: Tuple[str, str], delay: float) -> None:
        """Dynamically update edge weights based on traffic.

        Adds *delay* to the current weight of the edge, making congested
        paths less attractive to future A*/Dijkstra searches.
        """
        if self._graph is None:
            return
        from_id, to_id = edge_id
        current = self._graph.get_weight(from_id, to_id)
        if current is not None:
            self._graph.update_weight(from_id, to_id, current + delay)


class AllocationOptimizer:
    """
    Assigns spots to incoming requests using greedy allocation.

    Greedy: O(n*m) - assign each request to nearest available spot
    """

    def __init__(self):
        pass

    def allocate_greedy(
        self,
        requests: List[Tuple[str, Tuple[float, float]]],
        available_spots: List[Tuple[str, Tuple[float, float]]],
    ) -> AllocationResult:
        """Simple greedy - each request gets nearest available spot.

        For each request (in order), finds the closest remaining spot
        using Euclidean distance, assigns it, and removes that spot from
        the pool.
        """
        assignments: Dict[str, str] = {}
        unassigned: List[str] = []
        total_cost = 0.0
        remaining = list(available_spots)  # copy so we don't mutate caller's list

        for req_id, req_loc in requests:
            if not remaining:
                unassigned.append(req_id)
                continue

            # Find nearest spot by Euclidean distance
            best_idx = 0
            best_dist = float('inf')
            for i, (spot_id, spot_loc) in enumerate(remaining):
                dx = req_loc[0] - spot_loc[0]
                dy = req_loc[1] - spot_loc[1]
                dist = (dx * dx + dy * dy) ** 0.5
                if dist < best_dist:
                    best_dist = dist
                    best_idx = i

            assigned_spot_id, _ = remaining.pop(best_idx)
            assignments[req_id] = assigned_spot_id
            total_cost += best_dist

        return AllocationResult(
            assignments=assignments,
            total_cost=total_cost,
            unassigned=unassigned,
        )
