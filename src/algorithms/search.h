#pragma once

#include <string>
#include <vector>
#include <optional>
#include <limits>
#include <functional>
#include <algorithm>
#include <unordered_map>
#include <tuple>
#include <any>

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

class SpotSearcher {
public:
    SpotSearcher(QuadTree* spatial_index = nullptr,
                 std::function<bool(const std::string&)> availability_checker = nullptr)
        : _spatial_index(spatial_index)
        , _availability_checker(std::move(availability_checker)) {}

    std::vector<SearchResult> search(const SearchCriteria& criteria) {
        if (_spatial_index == nullptr) return {};

        double x = criteria.location.first;
        double y = criteria.location.second;

        std::vector<std::pair<Point, double>> candidates;
        if (criteria.max_distance < std::numeric_limits<double>::infinity()) {
            candidates = _spatial_index->query_radius(x, y, criteria.max_distance);
        } else {
            candidates = _spatial_index->k_nearest(x, y, criteria.max_results * 3);
        }

        std::vector<SearchResult> results;
        for (auto& [point, distance] : candidates) {
            auto [spot_id, zone_id, spot_type] = _unpack_point(point);

            if (_availability_checker && !_availability_checker(spot_id)) continue;

            if (criteria.spot_types.has_value()) {
                auto& types = criteria.spot_types.value();
                if (std::find(types.begin(), types.end(), spot_type) == types.end()) continue;
            }

            double score = _calculate_score(spot_id, distance, criteria);
            results.push_back({spot_id, distance, zone_id, score});
        }

        std::sort(results.begin(), results.end(),
                  [](const SearchResult& a, const SearchResult& b) { return a.score < b.score; });

        if (static_cast<int>(results.size()) > criteria.max_results) {
            results.resize(criteria.max_results);
        }
        return results;
    }

private:
    QuadTree* _spatial_index;
    std::function<bool(const std::string&)> _availability_checker;

    double _calculate_score(const std::string& spot_id, double distance,
                            const SearchCriteria& criteria) {
        return distance;
    }

    static std::tuple<std::string, std::string, std::string> _unpack_point(const Point& point) {
        try {
            auto& data = std::any_cast<const std::unordered_map<std::string, std::string>&>(point.data);
            auto sid_it = data.find("spot_id");
            auto zid_it = data.find("zone_id");
            auto stype_it = data.find("spot_type");
            return {
                sid_it != data.end() ? sid_it->second : "",
                zid_it != data.end() ? zid_it->second : "",
                stype_it != data.end() ? stype_it->second : "standard"
            };
        } catch (const std::bad_any_cast&) {
            return {"", "", "standard"};
        }
    }
};
