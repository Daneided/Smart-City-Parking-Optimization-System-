# Weighted graph for parking area navigation
# Used by RouteOptimizer for A*/Dijkstra pathfinding

from typing import Dict, List, Tuple, Optional, Set
import heapq


class Graph:
    """
    Weighted directed graph using adjacency list representation.

    Nodes represent locations (entrances, intersections, spots, exits).
    Edges represent drivable paths with distance/time weights.
    Supports dynamic weight updates for congestion modeling.
    """

    def __init__(self):
        self._adjacency: Dict[str, Dict[str, float]] = {}
        self._positions: Dict[str, Tuple[float, float]] = {}

    @property
    def nodes(self) -> Set[str]:
        return set(self._adjacency.keys())

    def add_node(self, node_id: str, position: Optional[Tuple[float, float]] = None) -> None:
        """Add a node. Position is (x, y) used as A* heuristic."""
        if node_id not in self._adjacency:
            self._adjacency[node_id] = {}
        if position is not None:
            self._positions[node_id] = position

    def add_edge(self, from_id: str, to_id: str, weight: float) -> None:
        """Add a directed edge. Creates nodes if they don't exist."""
        if from_id not in self._adjacency:
            self._adjacency[from_id] = {}
        if to_id not in self._adjacency:
            self._adjacency[to_id] = {}
        self._adjacency[from_id][to_id] = weight

    def add_edge_undirected(self, node_a: str, node_b: str, weight: float) -> None:
        """Add edges in both directions with the same weight."""
        self.add_edge(node_a, node_b, weight)
        self.add_edge(node_b, node_a, weight)

    def remove_edge(self, from_id: str, to_id: str) -> None:
        if from_id in self._adjacency:
            self._adjacency[from_id].pop(to_id, None)

    def remove_node(self, node_id: str) -> None:
        self._adjacency.pop(node_id, None)
        self._positions.pop(node_id, None)
        for neighbors in self._adjacency.values():
            neighbors.pop(node_id, None)

    def get_neighbors(self, node_id: str) -> List[Tuple[str, float]]:
        """Returns list of (neighbor_id, weight) for outgoing edges."""
        if node_id not in self._adjacency:
            return []
        return list(self._adjacency[node_id].items())

    def get_weight(self, from_id: str, to_id: str) -> Optional[float]:
        if from_id in self._adjacency:
            return self._adjacency[from_id].get(to_id)
        return None

    def update_weight(self, from_id: str, to_id: str, weight: float) -> bool:
        """Update edge weight (e.g. for congestion). Returns False if edge doesn't exist."""
        if from_id in self._adjacency and to_id in self._adjacency[from_id]:
            self._adjacency[from_id][to_id] = weight
            return True
        return False

    def get_position(self, node_id: str) -> Optional[Tuple[float, float]]:
        return self._positions.get(node_id)

    def has_node(self, node_id: str) -> bool:
        return node_id in self._adjacency

    def has_edge(self, from_id: str, to_id: str) -> bool:
        return from_id in self._adjacency and to_id in self._adjacency[from_id]

    def dijkstra(self, start: str, end: str) -> Optional[Tuple[List[str], float]]:
        """
        Shortest path using Dijkstra's algorithm.
        Returns (path, total_distance) or None if no path exists.
        """
        if start not in self._adjacency or end not in self._adjacency:
            return None

        dist: Dict[str, float] = {start: 0.0}
        prev: Dict[str, Optional[str]] = {start: None}
        # (distance, node_id)
        heap: List[Tuple[float, str]] = [(0.0, start)]
        visited: Set[str] = set()

        while heap:
            d, node = heapq.heappop(heap)
            if node in visited:
                continue
            visited.add(node)

            if node == end:
                return self._reconstruct_path(prev, end), d

            for neighbor, weight in self._adjacency[node].items():
                if neighbor in visited:
                    continue
                new_dist = d + weight
                if neighbor not in dist or new_dist < dist[neighbor]:
                    dist[neighbor] = new_dist
                    prev[neighbor] = node
                    heapq.heappush(heap, (new_dist, neighbor))

        return None

    def a_star(self, start: str, end: str) -> Optional[Tuple[List[str], float]]:
        """
        A* search using euclidean distance heuristic.
        Falls back to Dijkstra if positions are not set.
        """
        if start not in self._adjacency or end not in self._adjacency:
            return None

        end_pos = self._positions.get(end)
        if end_pos is None:
            return self.dijkstra(start, end)

        g_score: Dict[str, float] = {start: 0.0}
        prev: Dict[str, Optional[str]] = {start: None}
        # (f_score, node_id)
        heap: List[Tuple[float, str]] = [(self._heuristic(start, end_pos), start)]
        visited: Set[str] = set()

        while heap:
            _, node = heapq.heappop(heap)
            if node in visited:
                continue
            visited.add(node)

            if node == end:
                return self._reconstruct_path(prev, end), g_score[end]

            for neighbor, weight in self._adjacency[node].items():
                if neighbor in visited:
                    continue
                tentative_g = g_score[node] + weight
                if neighbor not in g_score or tentative_g < g_score[neighbor]:
                    g_score[neighbor] = tentative_g
                    prev[neighbor] = node
                    f = tentative_g + self._heuristic(neighbor, end_pos)
                    heapq.heappush(heap, (f, neighbor))

        return None

    def _heuristic(self, node: str, target_pos: Tuple[float, float]) -> float:
        pos = self._positions.get(node)
        if pos is None:
            return 0.0
        dx = pos[0] - target_pos[0]
        dy = pos[1] - target_pos[1]
        return (dx * dx + dy * dy) ** 0.5

    def _reconstruct_path(self, prev: Dict[str, Optional[str]], end: str) -> List[str]:
        path = []
        node: Optional[str] = end
        while node is not None:
            path.append(node)
            node = prev[node]
        path.reverse()
        return path
