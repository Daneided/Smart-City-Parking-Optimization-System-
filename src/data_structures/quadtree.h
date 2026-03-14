// QuadTree for spatial indexing of parking spots
// Supports range queries and k-nearest neighbor search

#ifndef QUADTREE_H
#define QUADTREE_H

#include <any>
#include <algorithm>
#include <cmath>
#include <memory>
#include <string>
#include <tuple>
#include <vector>

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

// QuadTree for 2D spatial indexing.
// Stores points and supports insert/remove, range query, and k-nearest neighbor search.
// Capacity: max points per leaf before subdivision.
class QuadTree {
public:
    QuadTree(const BoundingBox& boundary, int capacity = 4)
        : boundary(boundary), capacity(capacity), divided(false), _size(0) {}

    int size() const { return _size; }

    // Insert a point. Returns false if point is outside boundary.
    bool insert(const Point& point) {
        if (!boundary.contains(point.x, point.y)) {
            return false;
        }

        if (!divided && static_cast<int>(points.size()) < capacity) {
            points.push_back(point);
            _size++;
            return true;
        }

        if (!divided) {
            _subdivide();
        }

        if (nw->insert(point)) { _size++; return true; }
        if (ne->insert(point)) { _size++; return true; }
        if (sw->insert(point)) { _size++; return true; }
        if (se->insert(point)) { _size++; return true; }

        return false;
    }

    // Remove a point by coordinates and optional data match.
    bool remove(double x, double y) { return false; }

    // Find all points within the given bounding box.
    std::vector<Point> query_range(const BoundingBox& search_box) { return {}; }

    // Find all points within radius of (x, y). Returns (point, distance) pairs.
    std::vector<std::pair<Point, double>> query_radius(double x, double y, double radius) { return {}; }

    // Find k nearest points using branch-and-bound.
    std::vector<std::pair<Point, double>> k_nearest(double x, double y, int k = 1) { return {}; }

    BoundingBox boundary;
    int capacity;
    std::vector<Point> points;
    bool divided;
    std::unique_ptr<QuadTree> nw;
    std::unique_ptr<QuadTree> ne;
    std::unique_ptr<QuadTree> sw;
    std::unique_ptr<QuadTree> se;

private:
    int _size;

    void _subdivide() {
        auto [sw_b, se_b, nw_b, ne_b] = boundary.subdivide();
        sw = std::make_unique<QuadTree>(sw_b, capacity);
        se = std::make_unique<QuadTree>(se_b, capacity);
        nw = std::make_unique<QuadTree>(nw_b, capacity);
        ne = std::make_unique<QuadTree>(ne_b, capacity);
        divided = true;

        // Re-insert existing points into children
        std::vector<Point> remaining = std::move(points);
        points.clear();
        for (const auto& point : remaining) {
            bool inserted = nw->insert(point) || ne->insert(point) ||
                            sw->insert(point) || se->insert(point);
            if (!inserted) {
                // Edge case: point sits exactly on boundary, keep in parent
                points.push_back(point);
            }
        }
    }
    void _try_merge() {}
    void _query_range(const BoundingBox& search_box, std::vector<Point>& found) {}
    void _k_nearest(double x, double y, int k, std::vector<std::pair<double, Point>>& best) {}
};

#endif // QUADTREE_H
