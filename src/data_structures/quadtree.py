# QuadTree for spatial indexing of parking spots
# Supports range queries and k-nearest neighbor search

from typing import List, Tuple, Optional, Any


class Point:
    """A 2D point with associated data (e.g. spot_id)."""

    __slots__ = ('x', 'y', 'data')

    def __init__(self, x: float, y: float, data: Any = None):
        self.x = x
        self.y = y
        self.data = data

    def distance_to(self, other_x: float, other_y: float) -> float:
        dx = self.x - other_x
        dy = self.y - other_y
        return (dx * dx + dy * dy) ** 0.5

    def __repr__(self) -> str:
        return f"Point({self.x}, {self.y}, {self.data})"


class BoundingBox:
    """Axis-aligned bounding box defined by center and half-dimensions."""

    __slots__ = ('cx', 'cy', 'half_w', 'half_h')

    def __init__(self, cx: float, cy: float, half_w: float, half_h: float):
        self.cx = cx
        self.cy = cy
        self.half_w = half_w
        self.half_h = half_h

    def contains(self, x: float, y: float) -> bool:
        return (abs(x - self.cx) <= self.half_w and
                abs(y - self.cy) <= self.half_h)

    def intersects(self, other: 'BoundingBox') -> bool:
        return not (other.cx - other.half_w > self.cx + self.half_w or
                    other.cx + other.half_w < self.cx - self.half_w or
                    other.cy - other.half_h > self.cy + self.half_h or
                    other.cy + other.half_h < self.cy - self.half_h)

    def min_distance_to(self, x: float, y: float) -> float:
        """Minimum distance from a point to any point in this box."""
        dx = max(0.0, abs(x - self.cx) - self.half_w)
        dy = max(0.0, abs(y - self.cy) - self.half_h)
        return (dx * dx + dy * dy) ** 0.5

    def subdivide(self) -> Tuple['BoundingBox', 'BoundingBox', 'BoundingBox', 'BoundingBox']:
        qw = self.half_w / 2
        qh = self.half_h / 2
        return (
            BoundingBox(self.cx - qw, self.cy - qh, qw, qh),  # SW
            BoundingBox(self.cx + qw, self.cy - qh, qw, qh),  # SE
            BoundingBox(self.cx - qw, self.cy + qh, qw, qh),  # NW
            BoundingBox(self.cx + qw, self.cy + qh, qw, qh),  # NE
        )


class QuadTree:
    """
    QuadTree for 2D spatial indexing.

    Stores points and supports:
    - Insert / remove
    - Range query (all points within a bounding box)
    - K-nearest neighbor search with branch-and-bound pruning

    Capacity: max points per leaf before subdivision.
    """

    def __init__(self, boundary: BoundingBox, capacity: int = 4):
        self.boundary = boundary
        self.capacity = capacity
        self.points: List[Point] = []
        self.divided = False
        self.nw: Optional[QuadTree] = None
        self.ne: Optional[QuadTree] = None
        self.sw: Optional[QuadTree] = None
        self.se: Optional[QuadTree] = None
        self._size = 0

    @property
    def size(self) -> int:
        return self._size

    def insert(self, point: Point) -> bool:
        """Insert a point. Returns False if point is outside boundary."""
        if not self.boundary.contains(point.x, point.y):
            return False

        if not self.divided and len(self.points) < self.capacity:
            self.points.append(point)
            self._size += 1
            return True

        if not self.divided:
            self._subdivide()

        if self.nw.insert(point):
            self._size += 1
            return True
        if self.ne.insert(point):
            self._size += 1
            return True
        if self.sw.insert(point):
            self._size += 1
            return True
        if self.se.insert(point):
            self._size += 1
            return True

        return False

    def remove(self, x: float, y: float, data: Any = None) -> bool:
        """
        Remove a point by coordinates and optional data match.
        Returns True if a point was removed.
        """
        if not self.boundary.contains(x, y):
            return False

        for i, point in enumerate(self.points):
            if point.x == x and point.y == y:
                if data is not None and point.data != data:
                    continue
                self.points.pop(i)
                self._size -= 1
                return True

        if self.divided:
            for child in (self.nw, self.ne, self.sw, self.se):
                if child.remove(x, y, data):
                    self._size -= 1
                    self._try_merge()
                    return True

        return False

    def query_range(self, search_box: BoundingBox) -> List[Point]:
        """Find all points within the given bounding box."""
        found: List[Point] = []
        self._query_range(search_box, found)
        return found

    def _query_range(self, search_box: BoundingBox, found: List[Point]) -> None:
        if not self.boundary.intersects(search_box):
            return

        for point in self.points:
            if search_box.contains(point.x, point.y):
                found.append(point)

        if self.divided:
            self.nw._query_range(search_box, found)
            self.ne._query_range(search_box, found)
            self.sw._query_range(search_box, found)
            self.se._query_range(search_box, found)

    def query_radius(self, x: float, y: float, radius: float) -> List[Tuple[Point, float]]:
        """Find all points within radius of (x, y). Returns (point, distance) pairs."""
        # Use bounding box for coarse filter, then check exact distance
        search_box = BoundingBox(x, y, radius, radius)
        candidates = self.query_range(search_box)
        results = []
        r_sq = radius * radius
        for point in candidates:
            dx = point.x - x
            dy = point.y - y
            dist_sq = dx * dx + dy * dy
            if dist_sq <= r_sq:
                results.append((point, dist_sq ** 0.5))
        return results

    def k_nearest(self, x: float, y: float, k: int = 1) -> List[Tuple[Point, float]]:
        """
        Find k nearest points using branch-and-bound.

        Uses a bounded max-heap (via sorted list) and prunes subtrees
        whose minimum distance exceeds the current k-th best distance.

        Returns list of (point, distance) sorted by distance ascending.
        """
        # best stores (distance, point), kept sorted, max length k
        best: List[Tuple[float, Point]] = []
        self._k_nearest(x, y, k, best)
        return [(p, d) for d, p in best]

    def _k_nearest(self, x: float, y: float, k: int, best: List[Tuple[float, Point]]) -> None:
        min_dist = self.boundary.min_distance_to(x, y)
        if len(best) >= k and min_dist > best[-1][0]:
            return  # prune: this subtree can't beat current k-th best

        for point in self.points:
            dist = point.distance_to(x, y)
            if len(best) < k:
                best.append((dist, point))
                best.sort(key=lambda t: t[0])
            elif dist < best[-1][0]:
                best[-1] = (dist, point)
                best.sort(key=lambda t: t[0])

        if not self.divided:
            return

        # Visit children in order of proximity to query point for better pruning
        children = [
            (self.nw.boundary.min_distance_to(x, y), self.nw),
            (self.ne.boundary.min_distance_to(x, y), self.ne),
            (self.sw.boundary.min_distance_to(x, y), self.sw),
            (self.se.boundary.min_distance_to(x, y), self.se),
        ]
        children.sort(key=lambda t: t[0])

        for _, child in children:
            child._k_nearest(x, y, k, best)

    def _subdivide(self) -> None:
        sw_b, se_b, nw_b, ne_b = self.boundary.subdivide()
        self.sw = QuadTree(sw_b, self.capacity)
        self.se = QuadTree(se_b, self.capacity)
        self.nw = QuadTree(nw_b, self.capacity)
        self.ne = QuadTree(ne_b, self.capacity)
        self.divided = True

        # Re-insert existing points into children
        remaining = self.points
        self.points = []
        for point in remaining:
            inserted = (self.nw.insert(point) or self.ne.insert(point) or
                        self.sw.insert(point) or self.se.insert(point))
            if not inserted:
                # Edge case: point sits exactly on boundary, keep in parent
                self.points.append(point)

    def _try_merge(self) -> None:
        """Collapse children back into parent if total points fit in capacity."""
        if not self.divided:
            return

        total = (len(self.nw.points) + len(self.ne.points) +
                 len(self.sw.points) + len(self.se.points))

        # Only merge if none of the children are subdivided
        if (total <= self.capacity and
                not self.nw.divided and not self.ne.divided and
                not self.sw.divided and not self.se.divided):
            self.points.extend(self.nw.points)
            self.points.extend(self.ne.points)
            self.points.extend(self.sw.points)
            self.points.extend(self.se.points)
            self.nw = self.ne = self.sw = self.se = None
            self.divided = False
