// QuadTree for spatial indexing of parking spots
// Supports range queries and k-nearest neighbor search

#ifndef QUADTREE_H
#define QUADTREE_H

#include <any>
#include <cmath>
#include <string>

// A 2D point with associated data (e.g. spot_id)
struct Point {
    double x;
    double y;
    std::any data;

    Point(double x, double y, std::any data = std::any())
        : x(x), y(y), data(std::move(data)) {}

    double distance_to(double other_x, double other_y) const {
        double dx = x - other_x;
        double dy = y - other_y;
        return std::sqrt(dx * dx + dy * dy);
    }
};

#endif // QUADTREE_H
