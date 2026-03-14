#pragma once

#include <string>
#include <vector>
#include <optional>
#include <limits>
#include <functional>
#include <algorithm>

#include "../data_structures/quadtree.h"

struct SearchCriteria {
    std::pair<double, double> location;
    double max_distance = std::numeric_limits<double>::infinity();
    std::optional<std::vector<std::string>> spot_types = std::nullopt;
    int max_results = 10;
};

struct SearchResult {
    std::string spot_id;
    double distance;
    std::string zone_id;
    double score;
};
