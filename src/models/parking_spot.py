# Domain model for individual parking spots
# Tracks status with history and supports callback notifications

from dataclasses import dataclass, field
from datetime import datetime
from enum import Enum
from typing import Any, Callable, Dict, List, Optional, Tuple


class SpotStatus(Enum):
    """Possible states for a parking spot"""
    AVAILABLE = "available"
    OCCUPIED = "occupied"
    RESERVED = "reserved"
    MAINTENANCE = "maintenance"
    UNKNOWN = "unknown"


@dataclass
class ParkingSpot:
    """
    Core domain model representing a single parking spot.

    Integrates with the algorithm layer:
    - location feeds into QuadTree spatial indexing
    - status drives availability_checker via callback to AvailabilityTracker
    """
    spot_id: str
    location: Tuple[float, float]
    zone_id: str
    spot_type: str = "standard"
    status: SpotStatus = SpotStatus.AVAILABLE
    status_changed_at: Optional[datetime] = None
    metadata: Dict[str, Any] = field(default_factory=dict)
    _status_history: List[Tuple[SpotStatus, datetime]] = field(
        default_factory=list, repr=False
    )
    _on_status_change: Optional[Callable] = field(default=None, repr=False)

    @property
    def is_available(self) -> bool:
        return self.status == SpotStatus.AVAILABLE

    @property
    def x(self) -> float:
        return self.location[0]

    @property
    def y(self) -> float:
        return self.location[1]

    def change_status(self, new_status: SpotStatus, timestamp: Optional[datetime] = None) -> None:
        """Update spot status, record history, and fire callback."""
        if new_status == self.status:
            return

        old_status = self.status
        ts = timestamp or datetime.now()

        self.status = new_status
        self.status_changed_at = ts
        self._status_history.append((new_status, ts))

        if self._on_status_change is not None:
            self._on_status_change(self.spot_id, old_status, new_status)

    def occupy(self, timestamp: Optional[datetime] = None) -> None:
        self.change_status(SpotStatus.OCCUPIED, timestamp)

    def release(self, timestamp: Optional[datetime] = None) -> None:
        self.change_status(SpotStatus.AVAILABLE, timestamp)

    def reserve(self, timestamp: Optional[datetime] = None) -> None:
        self.change_status(SpotStatus.RESERVED, timestamp)

    def set_maintenance(self, timestamp: Optional[datetime] = None) -> None:
        self.change_status(SpotStatus.MAINTENANCE, timestamp)

    def get_status_history(self) -> List[Tuple[SpotStatus, datetime]]:
        """Return a copy of the status history."""
        return list(self._status_history)

    def duration_in_current_status(self) -> float:
        """Seconds spent in the current status, or 0.0 if no change recorded."""
        if self.status_changed_at is None:
            return 0.0
        return (datetime.now() - self.status_changed_at).total_seconds()

    def register_status_callback(self, callback: Callable) -> None:
        """Set the callback invoked on every status change."""
        self._on_status_change = callback
