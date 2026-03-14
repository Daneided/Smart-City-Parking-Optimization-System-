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
                       bool available = true) {
        _spot_zones[spot_id] = zone_id;

        if (_available_by_zone.find(zone_id) == _available_by_zone.end()) {
            _available_by_zone[zone_id] = {};
            _zone_totals[zone_id] = 0;
        }

        _zone_totals[zone_id] += 1;

        if (available) {
            _available_all.insert(spot_id);
            _available_by_zone[zone_id].insert(spot_id);
        }
    }

    void unregister_spot(const std::string& spot_id) {
        auto it = _spot_zones.find(spot_id);
        std::string zone_id;
        if (it != _spot_zones.end()) {
            zone_id = it->second;
            _spot_zones.erase(it);
        }

        _available_all.erase(spot_id);

        if (!zone_id.empty()) {
            auto zone_it = _available_by_zone.find(zone_id);
            if (zone_it != _available_by_zone.end()) {
                zone_it->second.erase(spot_id);
            }
            auto total_it = _zone_totals.find(zone_id);
            if (total_it != _zone_totals.end()) {
                total_it->second -= 1;
            }
        }
    }

    void on_status_change(const std::string& spot_id, SpotStatus old_status,
                          SpotStatus new_status) {
        auto it = _spot_zones.find(spot_id);
        if (it == _spot_zones.end()) return;

        const std::string& zone_id = it->second;

        if (new_status == SpotStatus::AVAILABLE) {
            _available_all.insert(spot_id);
            _available_by_zone[zone_id].insert(spot_id);
        } else {
            _available_all.erase(spot_id);
            _available_by_zone[zone_id].erase(spot_id);
        }
    }

    bool is_available(const std::string& spot_id) const {
        return _available_all.count(spot_id) > 0;
    }

    std::unordered_set<std::string> get_available_in_zone(const std::string& zone_id) const {
        auto it = _available_by_zone.find(zone_id);
        if (it != _available_by_zone.end()) return it->second;
        return {};
    }

    std::unordered_set<std::string> get_all_available() const {
        return _available_all;
    }

    int count_available(std::optional<std::string> zone_id = std::nullopt) const {
        if (zone_id.has_value()) {
            auto it = _available_by_zone.find(zone_id.value());
            if (it != _available_by_zone.end()) return static_cast<int>(it->second.size());
            return 0;
        }
        return static_cast<int>(_available_all.size());
    }

    double get_zone_occupancy_rate(const std::string& zone_id) const {
        auto it = _zone_totals.find(zone_id);
        if (it == _zone_totals.end() || it->second == 0) return 0.0;

        int total = it->second;
        int available = 0;
        auto avail_it = _available_by_zone.find(zone_id);
        if (avail_it != _available_by_zone.end()) {
            available = static_cast<int>(avail_it->second.size());
        }
        return 1.0 - (static_cast<double>(available) / total);
    }

private:
    std::unordered_set<std::string> _available_all;
    std::unordered_map<std::string, std::unordered_set<std::string>> _available_by_zone;
    std::unordered_map<std::string, std::string> _spot_zones;
    std::unordered_map<std::string, int> _zone_totals;
};
