#pragma once

#include <string>

struct Coordinate {
    double x;
    double y;
};

struct SpotData {
    std::string spot_id;
    std::string zone_id;
    std::string spot_type;
};
