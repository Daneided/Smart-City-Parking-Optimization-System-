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

class SpotSearcher {
public:
    SpotSearcher(QuadTree* spatial_index = nullptr,
                 std::function<bool(const std::string&)> availability_checker = nullptr)
        : _spatial_index(spatial_index)
        , _availability_checker(std::move(availability_checker)) {}

    std::optional<SearchResult> search_nearest(
            std::pair<double, double> location,
            std::optional<std::string> spot_type = std::nullopt) {
        if (_spatial_index == nullptr) return std::nullopt;

        int tree_size = _spatial_index->size();
        if (tree_size == 0) return std::nullopt;

        double x = location.first;
        double y = location.second;
        int k = std::min(10, tree_size);

        while (true) {
            auto candidates = _spatial_index->k_nearest(x, y, k);
            for (auto& [point, distance] : candidates) {
                const SpotData& d = point.data;

                if (_availability_checker && !_availability_checker(d.spot_id)) continue;
                if (spot_type.has_value() && d.spot_type != spot_type.value()) continue;

                return SearchResult{d.spot_id, distance, d.zone_id, distance};
            }

            if (k >= tree_size) return std::nullopt;
            k = std::min(k * 2, tree_size);
        }
    }

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
            const SpotData& d = point.data;

            if (_availability_checker && !_availability_checker(d.spot_id)) continue;

            if (criteria.spot_types.has_value()) {
                auto& types = criteria.spot_types.value();
                if (std::find(types.begin(), types.end(), d.spot_type) == types.end()) continue;
            }

            double score = _calculate_score(d.spot_id, distance, criteria);
            results.push_back({d.spot_id, distance, d.zone_id, score});
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
};
