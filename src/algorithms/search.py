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

    Time complexity: O(log n + k) average, O(n) worst case
    """

    def __init__(self, spatial_index=None, availability_checker=None):
        # TODO: store dependencies for spatial queries and availability checks
        self._spatial_index = spatial_index
        self._availability_checker = availability_checker

    def search(self, criteria: SearchCriteria) -> List[SearchResult]:
        """
        Main search method:
        1. Query spatial index for candidates in range
        2. Filter by availability and type
        3. Score and rank results
        """
        # TODO: implement search logic
        pass

    def search_nearest(self, location: Tuple[float, float], spot_type: Optional[str] = None) -> Optional[SearchResult]:
        """Quick method to find single nearest available spot"""
        # TODO: implement using k=1 search
        pass

    def _calculate_score(self, spot_id: str, distance: float, criteria: SearchCriteria) -> float:
        """
        Scoring function - lower is better
        Factors: distance, type match, zone pricing (future)
        """
        # TODO: implement scoring algorithm
        pass
