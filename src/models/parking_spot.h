#pragma once

#include <string>
#include <chrono>
#include <vector>
#include <functional>
#include <optional>
#include <utility>

enum class SpotStatus {
    AVAILABLE,
    OCCUPIED,
    RESERVED,
    MAINTENANCE,
    UNKNOWN
};

inline std::string spot_status_to_string(SpotStatus status) {
    switch (status) {
        case SpotStatus::AVAILABLE:   return "available";
        case SpotStatus::OCCUPIED:    return "occupied";
        case SpotStatus::RESERVED:    return "reserved";
        case SpotStatus::MAINTENANCE: return "maintenance";
        case SpotStatus::UNKNOWN:     return "unknown";
    }
    return "unknown";
}

inline SpotStatus string_to_spot_status(const std::string& s) {
    if (s == "available")   return SpotStatus::AVAILABLE;
    if (s == "occupied")    return SpotStatus::OCCUPIED;
    if (s == "reserved")    return SpotStatus::RESERVED;
    if (s == "maintenance") return SpotStatus::MAINTENANCE;
    return SpotStatus::UNKNOWN;
}

using TimePoint = std::chrono::system_clock::time_point;
using StatusCallback = std::function<void(const std::string&, SpotStatus, SpotStatus)>;

class ParkingSpot {
public:
    std::string spot_id;
    std::pair<double, double> location;
    std::string zone_id;
    std::string spot_type;
    SpotStatus status;
    std::optional<TimePoint> status_changed_at;

    ParkingSpot(const std::string& spot_id,
                std::pair<double, double> location,
                const std::string& zone_id,
                const std::string& spot_type = "standard",
                SpotStatus status = SpotStatus::AVAILABLE)
        : spot_id(spot_id)
        , location(location)
        , zone_id(zone_id)
        , spot_type(spot_type)
        , status(status)
        , status_changed_at(std::nullopt)
        , _on_status_change(nullptr) {}

    bool is_available() const { return status == SpotStatus::AVAILABLE; }
    double x() const { return location.first; }
    double y() const { return location.second; }

    void change_status(SpotStatus new_status,
                       std::optional<TimePoint> timestamp = std::nullopt) {
        if (new_status == status) return;

        SpotStatus old_status = status;
        TimePoint ts = timestamp.value_or(std::chrono::system_clock::now());

        status = new_status;
        status_changed_at = ts;
        _status_history.emplace_back(new_status, ts);

        if (_on_status_change) {
            _on_status_change(spot_id, old_status, new_status);
        }
    }

    void occupy(std::optional<TimePoint> timestamp = std::nullopt) {
        change_status(SpotStatus::OCCUPIED, timestamp);
    }

    void release(std::optional<TimePoint> timestamp = std::nullopt) {
        change_status(SpotStatus::AVAILABLE, timestamp);
    }

    void reserve(std::optional<TimePoint> timestamp = std::nullopt) {
        change_status(SpotStatus::RESERVED, timestamp);
    }

    void set_maintenance(std::optional<TimePoint> timestamp = std::nullopt) {
        change_status(SpotStatus::MAINTENANCE, timestamp);
    }

    std::vector<std::pair<SpotStatus, TimePoint>> get_status_history() const {
        return _status_history;
    }

    void register_status_callback(StatusCallback callback) {
        _on_status_change = std::move(callback);
    }

private:
    std::vector<std::pair<SpotStatus, TimePoint>> _status_history;
    StatusCallback _on_status_change;
};
