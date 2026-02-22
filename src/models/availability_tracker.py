# Set-based availability tracker with O(1) lookups
# Designed to plug directly into algorithm layer as availability_checker

from typing import Dict, Optional, Set


class AvailabilityTracker:
    """
    Tracks which parking spots are available using set membership.

    Core design:
    - is_available(spot_id) -> bool matches Callable[[str], bool]
      expected by SpotSearcher and NearestSpotFinder
    - Dual indexing: global set + per-zone sets for both
      spot-level and zone-level queries
    """

    def __init__(self) -> None:
        self._available_all: Set[str] = set()
        self._available_by_zone: Dict[str, Set[str]] = {}
        self._spot_zones: Dict[str, str] = {}
        self._zone_totals: Dict[str, int] = {}

    def register_spot(self, spot_id: str, zone_id: str, is_available: bool = True) -> None:
        """Add a spot to tracking."""
        self._spot_zones[spot_id] = zone_id

        if zone_id not in self._available_by_zone:
            self._available_by_zone[zone_id] = set()
            self._zone_totals[zone_id] = 0

        self._zone_totals[zone_id] += 1

        if is_available:
            self._available_all.add(spot_id)
            self._available_by_zone[zone_id].add(spot_id)

    def unregister_spot(self, spot_id: str) -> None:
        """Remove a spot from tracking."""
        zone_id = self._spot_zones.pop(spot_id, None)
        self._available_all.discard(spot_id)

        if zone_id is not None:
            zone_set = self._available_by_zone.get(zone_id)
            if zone_set is not None:
                zone_set.discard(spot_id)
            if zone_id in self._zone_totals:
                self._zone_totals[zone_id] -= 1

    def on_status_change(self, spot_id: str, old_status: object, new_status: object) -> None:
        """
        Callback for ParkingSpot.change_status().

        Checks new_status.value instead of importing SpotStatus
        to avoid circular dependencies.
        """
        zone_id = self._spot_zones.get(spot_id)
        if zone_id is None:
            return

        if getattr(new_status, 'value', None) == "available":
            self._available_all.add(spot_id)
            self._available_by_zone[zone_id].add(spot_id)
        else:
            self._available_all.discard(spot_id)
            self._available_by_zone[zone_id].discard(spot_id)

    def is_available(self, spot_id: str) -> bool:
        """O(1) availability check — matches Callable[[str], bool]."""
        return spot_id in self._available_all

    def get_available_in_zone(self, zone_id: str) -> Set[str]:
        """Return a copy of available spot IDs in the given zone."""
        return set(self._available_by_zone.get(zone_id, set()))

    def get_all_available(self) -> Set[str]:
        """Return a copy of all available spot IDs."""
        return set(self._available_all)

    def count_available(self, zone_id: Optional[str] = None) -> int:
        """Count available spots, optionally filtered by zone."""
        if zone_id is not None:
            return len(self._available_by_zone.get(zone_id, set()))
        return len(self._available_all)

    def get_zone_occupancy_rate(self, zone_id: str) -> float:
        """Fraction of spots occupied in a zone (0.0–1.0)."""
        total = self._zone_totals.get(zone_id, 0)
        if total == 0:
            return 0.0
        available = len(self._available_by_zone.get(zone_id, set()))
        return 1.0 - (available / total)
