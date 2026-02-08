# Search algorithms for finding available parking spots
# This is the core functionality users will interact with

from typing import List, Tuple, Optional, Callable
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


class NearestSpotFinder:
    """
    Optimized k-nearest neighbor search using branch-and-bound with QuadTree.
    More efficient than brute force when we only need a few results.
    
    Algorithm:
    - BFS from QuadTree root using priority queue
    - Prune subtrees that can't have closer spots
    - Stop when k available spots found
    """
    
    def __init__(self, spatial_index=None):
        self._spatial_index = spatial_index
    
    def find_k_nearest(self, location: Tuple[float, float], k: int = 5, filter_fn: Optional[Callable] = None) -> List[Tuple[str, float]]:
        """Returns list of (spot_id, distance) sorted by distance"""
        # TODO: implement k-NN with spatial pruning
        pass
    
    def find_nearest_available(self, location: Tuple[float, float], availability_checker: Callable[[str], bool]) -> Optional[Tuple[str, float]]:
        """Expands search until we find an available spot"""
        # TODO: implement incremental nearest search
        pass


class ZoneSearcher:
    """
    Higher-level search operating on zones instead of individual spots.
    Useful for queries like "find parking near downtown"
    
    Future extension - not needed for initial implementation
    """
    
    def __init__(self):
        pass
    
    def find_zones_with_availability(self, location: Tuple[float, float], min_spots: int = 1) -> List[Tuple[str, int, float]]:
        """Returns (zone_id, available_count, distance) tuples"""
        # TODO: implement zone-level search
        pass
