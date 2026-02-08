# Demand prediction and forecasting
# Uses historical data to predict future parking availability

from typing import List, Tuple, Dict, Optional
from dataclasses import dataclass
from datetime import datetime, timedelta


@dataclass
class PredictionResult:
    """Single prediction with confidence interval"""
    timestamp: datetime
    predicted_value: float
    confidence: Tuple[float, float]
    model_used: str


@dataclass
class DemandForecast:
    """Multi-hour demand forecast for a zone"""
    zone_id: str
    start_time: datetime
    end_time: datetime
    hourly_demand: List[float]


class DemandPredictor:
    """
    Predicts parking demand from historical patterns.
    
    Implemented without ML libraries - uses basic stats:
    - Moving averages
    - Seasonal decomposition (day of week, hour)
    - Simple linear regression for trends
    """
    
    def __init__(self, history_window: int = 7):
        self.history_window = history_window
        self._hourly_averages: Dict[Tuple[int, int], float] = {}  # (day, hour) -> avg
        self._daily_patterns: Dict[int, List[float]] = {}
    
    def train(self, historical_data: List[Dict]) -> None:
        """
        Build model from historical occupancy data.
        Groups by day/hour and calculates averages.
        """
        # TODO: calculate rolling averages and patterns
        pass
    
    def predict(self, zone_id: str, target_time: datetime) -> PredictionResult:
        """Predict occupancy at a specific future time"""
        # TODO: combine base pattern with trend adjustment
        pass
    
    def forecast(self, zone_id: str, start_time: datetime, hours: int = 24) -> DemandForecast:
        """Generate hourly predictions for a time range"""
        # TODO: call predict() for each hour
        pass
    
    def _calculate_seasonal_component(self, day_of_week: int, hour: int) -> float:
        """Adjustment factor based on typical patterns (weekday vs weekend, peak hours)"""
        # TODO: implement seasonal decomposition
        pass
    
    def _calculate_trend(self, recent_values: List[float]) -> float:
        """Linear regression on recent data to detect trends"""
        # TODO: implement least squares regression
        pass


class OccupancyForecaster:
    """
    Predicts when spots will be available (inverse of demand).
    
    Use cases:
    - "Best time to find parking"
    - Suggesting reservation time slots
    """
    
    def __init__(self):
        pass
    
    def predict_availability(self, spot_id: str, time_range: Tuple[datetime, datetime]) -> List[Tuple[datetime, float]]:
        """Returns (timestamp, probability) pairs for spot availability"""
        # TODO: model availability as function of time
        pass
    
    def find_best_time(self, zone_id: str, search_window: Tuple[datetime, datetime], min_spots: int = 1) -> Optional[datetime]:
        """Find optimal time within window to have min_spots available"""
        # TODO: search for availability peaks
        pass
    
    def estimate_wait_time(self, zone_id: str, current_time: datetime) -> Optional[timedelta]:
        """How long until a spot opens up?"""
        # TODO: estimate based on turnover rate
        pass


class StatisticsCalculator:
    """
    Basic stats helper - implementing from scratch per project requirements.
    No numpy/scipy allowed for core logic.
    """
    
    @staticmethod
    def mean(values: List[float]) -> float:
        # TODO: sum / len
        pass
    
    @staticmethod
    def variance(values: List[float]) -> float:
        # TODO: sum of squared differences from mean
        pass
    
    @staticmethod
    def std_dev(values: List[float]) -> float:
        # TODO: sqrt of variance
        pass
    
    @staticmethod
    def linear_regression(x: List[float], y: List[float]) -> Tuple[float, float]:
        """
        Returns (slope, intercept) using least squares.
        slope = Σ(xi - x̄)(yi - ȳ) / Σ(xi - x̄)²
        """
        # TODO: implement least squares formula
        pass
    
    @staticmethod
    def moving_average(values: List[float], window: int) -> List[float]:
        # TODO: sliding window average
        pass
    
    @staticmethod
    def exponential_moving_average(values: List[float], alpha: float = 0.3) -> List[float]:
        """EMA gives more weight to recent values"""
        # TODO: implement EMA formula
        pass
