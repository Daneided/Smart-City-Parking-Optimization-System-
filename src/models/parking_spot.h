#pragma once

#include <string>
#include <chrono>
#include <vector>
#include <functional>
#include <optional>
#include <utility>
#include <unordered_map>

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
