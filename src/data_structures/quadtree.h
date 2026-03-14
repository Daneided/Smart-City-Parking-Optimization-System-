// QuadTree for spatial indexing of parking spots
// Supports range queries and k-nearest neighbor search

#ifndef QUADTREE_H
#define QUADTREE_H

#include <any>
#include <algorithm>
#include <cmath>
#include <string>
#include <tuple>

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

// Axis-aligned bounding box defined by center and half-dimensions
struct BoundingBox {
    double cx;
    double cy;
    double half_w;
    double half_h;

    BoundingBox(double cx, double cy, double half_w, double half_h)
        : cx(cx), cy(cy), half_w(half_w), half_h(half_h) {}

    bool contains(double x, double y) const {
        return std::abs(x - cx) <= half_w &&
               std::abs(y - cy) <= half_h;
    }

    bool intersects(const BoundingBox& other) const {
        return !(other.cx - other.half_w > cx + half_w ||
                 other.cx + other.half_w < cx - half_w ||
                 other.cy - other.half_h > cy + half_h ||
                 other.cy + other.half_h < cy - half_h);
    }

    // Minimum distance from a point to any point in this box
    double min_distance_to(double x, double y) const {
        double dx = std::max(0.0, std::abs(x - cx) - half_w);
        double dy = std::max(0.0, std::abs(y - cy) - half_h);
        return std::sqrt(dx * dx + dy * dy);
    }

    std::tuple<BoundingBox, BoundingBox, BoundingBox, BoundingBox> subdivide() const {
        double qw = half_w / 2.0;
        double qh = half_h / 2.0;
        return {
            BoundingBox(cx - qw, cy - qh, qw, qh),  // SW
            BoundingBox(cx + qw, cy - qh, qw, qh),  // SE
            BoundingBox(cx - qw, cy + qh, qw, qh),  // NW
            BoundingBox(cx + qw, cy + qh, qw, qh),  // NE
        };
    }
};

#endif // QUADTREE_H
