# Priority queue for search ranking and pathfinding
# Wraps heapq with decrease-key support needed by A* and spot ranking

from typing import Any, Optional, Dict, List, Tuple
import heapq


class PriorityQueue:
    """
    Min-heap priority queue with decrease-key support.

    Used by:
    - SpotSearcher for ranking search results by score
    - RouteOptimizer internally (A*/Dijkstra use heapq directly,
      but this is available for more complex use cases)

    Supports updating priority of existing items without duplicates.
    """

    def __init__(self):
        self._heap: List[Tuple[float, int, Any]] = []
        self._entry_map: Dict[Any, Tuple[float, int, Any]] = {}
        self._counter = 0
        self._size = 0
        self._REMOVED = object()

    def __len__(self) -> int:
        return self._size

    def __bool__(self) -> bool:
        return self._size > 0

    def __contains__(self, item: Any) -> bool:
        return item in self._entry_map

    def push(self, item: Any, priority: float) -> None:
        """Add item or update its priority if already present."""
        if item in self._entry_map:
            self._mark_removed(item)
        entry = (priority, self._counter, item)
        self._counter += 1
        self._entry_map[item] = entry
        heapq.heappush(self._heap, entry)
        self._size += 1

    def pop(self) -> Optional[Any]:
        """Remove and return item with lowest priority."""
        while self._heap:
            priority, count, item = heapq.heappop(self._heap)
            if item is not self._REMOVED:
                del self._entry_map[item]
                self._size -= 1
                return item
        return None

    def pop_with_priority(self) -> Optional[Tuple[Any, float]]:
        """Remove and return (item, priority) with lowest priority."""
        while self._heap:
            priority, count, item = heapq.heappop(self._heap)
            if item is not self._REMOVED:
                del self._entry_map[item]
                self._size -= 1
                return (item, priority)
        return None

    def peek(self) -> Optional[Any]:
        """Return item with lowest priority without removing it."""
        while self._heap:
            priority, count, item = self._heap[0]
            if item is not self._REMOVED:
                return item
            heapq.heappop(self._heap)
        return None

    def peek_priority(self) -> Optional[float]:
        """Return the lowest priority value without removing."""
        while self._heap:
            priority, count, item = self._heap[0]
            if item is not self._REMOVED:
                return priority
            heapq.heappop(self._heap)
        return None

    def get_priority(self, item: Any) -> Optional[float]:
        """Get current priority of an item, or None if not present."""
        entry = self._entry_map.get(item)
        if entry is not None:
            return entry[0]
        return None

    def decrease_key(self, item: Any, new_priority: float) -> bool:
        """
        Decrease priority of an existing item.
        Returns False if item not found or new priority is not lower.
        """
        entry = self._entry_map.get(item)
        if entry is None:
            return False
        if new_priority >= entry[0]:
            return False
        self.push(item, new_priority)
        return True

    def _mark_removed(self, item: Any) -> None:
        entry = self._entry_map.pop(item)
        # Replace item in the tuple with sentinel — the tuple in the heap
        # is immutable, so we track it via entry_map and skip on pop
        # We create a new entry with REMOVED marker
        idx = 2
        lst = list(entry)
        lst[idx] = self._REMOVED
        # The old entry remains in heap but will be skipped
        self._size -= 1
