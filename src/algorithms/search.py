# Search algorithms for finding available parking spots
# This is the core functionality users will interact with

from typing import List, Tuple, Optional
from dataclasses import dataclass


@dataclass
class SearchCriteria:
    """Parameters for spot search - location, distance limits, spot type preferences"""
    location: Tuple[float, float]
    max_distance: float = float('inf')
    spot_types: Optional[List[str]] = None
    max_results: int = 10


@dataclass
class SearchResult:
    """Single search result with spot info and distance"""
    spot_id: str
    distance: float
    zone_id: str
    score: float


class SpotSearcher:
    """
    Main search interface for finding parking spots.
    Uses spatial indexing (QuadTree) and priority queue for ranking.

    Expects Point.data to be a dict with keys: spot_id, zone_id, spot_type.

    Time complexity: O(log n + k) average, O(n) worst case
    """

    def __init__(self, spatial_index=None, availability_checker=None):
        self._spatial_index = spatial_index
        self._availability_checker = availability_checker

    def search(self, criteria: SearchCriteria) -> List[SearchResult]:
        """
        Main search method:
        1. Query spatial index for candidates in range
        2. Filter by availability and type
        3. Score and rank results
        """
        if self._spatial_index is None:
            return []

        x, y = criteria.location

        # Get candidates from spatial index
        if criteria.max_distance < float('inf'):
            candidates = self._spatial_index.query_radius(x, y, criteria.max_distance)
        else:
            # No distance limit — fetch a larger pool to allow for filtering
            candidates = self._spatial_index.k_nearest(x, y, k=criteria.max_results * 3)

        results: List[SearchResult] = []
        for point, distance in candidates:
            spot_id, zone_id, spot_type = self._unpack_point(point)

            # Filter by availability
            if self._availability_checker is not None and not self._availability_checker(spot_id):
                continue

            # Filter by spot type
            if criteria.spot_types is not None and spot_type not in criteria.spot_types:
                continue

            score = self._calculate_score(spot_id, distance, criteria)
            results.append(SearchResult(
                spot_id=spot_id, distance=distance, zone_id=zone_id, score=score,
            ))

        # Sort by score (lower is better) and limit results
        results.sort(key=lambda r: r.score)
        return results[:criteria.max_results]

    def search_nearest(self, location: Tuple[float, float], spot_type: Optional[str] = None) -> Optional[SearchResult]:
        """Quick method to find single nearest available spot.

        Uses k-nearest with expanding k until an available spot is found.
        """
        if self._spatial_index is None:
            return None

        x, y = location
        tree_size = getattr(self._spatial_index, 'size', 0) or 0
        if tree_size == 0:
            return None

        k = min(10, tree_size)
        while True:
            candidates = self._spatial_index.k_nearest(x, y, k=k)
            for point, distance in candidates:
                sid, zid, stype = self._unpack_point(point)

                if self._availability_checker is not None and not self._availability_checker(sid):
                    continue
                if spot_type is not None and stype != spot_type:
                    continue

                return SearchResult(spot_id=sid, distance=distance, zone_id=zid, score=distance)

            # Already searched the whole tree
            if k >= tree_size:
                return None
            k = min(k * 2, tree_size)

    def _calculate_score(self, spot_id: str, distance: float, criteria: SearchCriteria) -> float:
        """
        Scoring function — lower is better.
        Currently distance-based; extend later with pricing / preferences.
        """
        return distance

    @staticmethod
    def _unpack_point(point) -> Tuple[str, str, str]:
        """Extract (spot_id, zone_id, spot_type) from a QuadTree Point."""
        data = point.data
        if isinstance(data, dict):
            return data.get("spot_id", ""), data.get("zone_id", ""), data.get("spot_type", "standard")
        return str(data), "", "standard"
