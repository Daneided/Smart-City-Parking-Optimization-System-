#pragma once

#include <string>
#include <unordered_set>
#include <unordered_map>
#include <optional>

#include "parking_spot.h"

class AvailabilityTracker {
public:
    AvailabilityTracker() = default;

    void register_spot(const std::string& spot_id, const std::string& zone_id,
                       bool is_available = true);
    void unregister_spot(const std::string& spot_id);
    void on_status_change(const std::string& spot_id, SpotStatus old_status,
                          SpotStatus new_status);

    bool is_available(const std::string& spot_id) const;
    std::unordered_set<std::string> get_available_in_zone(const std::string& zone_id) const;
    std::unordered_set<std::string> get_all_available() const;
    int count_available(std::optional<std::string> zone_id = std::nullopt) const;
    double get_zone_occupancy_rate(const std::string& zone_id) const;

private:
    std::unordered_set<std::string> _available_all;
    std::unordered_map<std::string, std::unordered_set<std::string>> _available_by_zone;
    std::unordered_map<std::string, std::string> _spot_zones;
    std::unordered_map<std::string, int> _zone_totals;
};
