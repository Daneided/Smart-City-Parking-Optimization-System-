#pragma once

#include <cmath>
#include <string>

struct Coordinate {
    double x;
    double y;

    double distance_to(const Coordinate& other) const {
        double dx = x - other.x;
        double dy = y - other.y;
        return std::sqrt(dx * dx + dy * dy);
    }
};

struct SpotData {
    std::string spot_id;
    std::string zone_id;
    std::string spot_type;
};
