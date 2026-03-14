// QuadTree tests — BoundingBox contains/intersects

#include "test_framework.h"
#include "../src/data_structures/quadtree.h"

// --- BoundingBox::contains ---

TEST(BoundingBox, ContainsPointInside) {
    BoundingBox box(5.0, 5.0, 5.0, 5.0);  // covers [0,10] x [0,10]
    ASSERT_TRUE(box.contains(5.0, 5.0));   // center
    ASSERT_TRUE(box.contains(1.0, 1.0));   // interior
    ASSERT_TRUE(box.contains(9.0, 9.0));   // interior
}

TEST(BoundingBox, ContainsPointOnEdge) {
    BoundingBox box(5.0, 5.0, 5.0, 5.0);
    ASSERT_TRUE(box.contains(0.0, 0.0));   // corner
    ASSERT_TRUE(box.contains(10.0, 10.0)); // corner
    ASSERT_TRUE(box.contains(5.0, 0.0));   // edge midpoint
    ASSERT_TRUE(box.contains(0.0, 5.0));   // edge midpoint
}

TEST(BoundingBox, ContainsRejectsOutside) {
    BoundingBox box(5.0, 5.0, 5.0, 5.0);
    ASSERT_FALSE(box.contains(-0.1, 5.0));
    ASSERT_FALSE(box.contains(10.1, 5.0));
    ASSERT_FALSE(box.contains(5.0, -0.1));
    ASSERT_FALSE(box.contains(5.0, 10.1));
    ASSERT_FALSE(box.contains(100.0, 100.0));
}

// --- BoundingBox::intersects ---

TEST(BoundingBox, IntersectsOverlapping) {
    BoundingBox a(5.0, 5.0, 5.0, 5.0);   // [0,10] x [0,10]
    BoundingBox b(8.0, 8.0, 5.0, 5.0);   // [3,13] x [3,13]
    ASSERT_TRUE(a.intersects(b));
    ASSERT_TRUE(b.intersects(a));          // symmetric
}

TEST(BoundingBox, IntersectsTouching) {
    BoundingBox a(5.0, 5.0, 5.0, 5.0);   // [0,10] x [0,10]
    BoundingBox b(15.0, 5.0, 5.0, 5.0);  // [10,20] x [0,10]
    ASSERT_TRUE(a.intersects(b));          // share edge at x=10
}

TEST(BoundingBox, IntersectsContained) {
    BoundingBox outer(5.0, 5.0, 5.0, 5.0);
    BoundingBox inner(5.0, 5.0, 1.0, 1.0);  // small box inside
    ASSERT_TRUE(outer.intersects(inner));
    ASSERT_TRUE(inner.intersects(outer));
}

TEST(BoundingBox, IntersectsDisjoint) {
    BoundingBox a(0.0, 0.0, 1.0, 1.0);   // [-1,1] x [-1,1]
    BoundingBox b(5.0, 5.0, 1.0, 1.0);   // [4,6] x [4,6]
    ASSERT_FALSE(a.intersects(b));
    ASSERT_FALSE(b.intersects(a));
}

// --- BoundingBox::min_distance_to ---

TEST(BoundingBox, MinDistanceInsideIsZero) {
    BoundingBox box(5.0, 5.0, 5.0, 5.0);
    ASSERT_NEAR(box.min_distance_to(5.0, 5.0), 0.0, 1e-9);
    ASSERT_NEAR(box.min_distance_to(3.0, 7.0), 0.0, 1e-9);
}

TEST(BoundingBox, MinDistanceOutside) {
    BoundingBox box(5.0, 5.0, 5.0, 5.0);  // [0,10] x [0,10]
    // Point at (13, 5) — 3 units right of box edge
    ASSERT_NEAR(box.min_distance_to(13.0, 5.0), 3.0, 1e-9);
    // Point at (0, -4) — 4 units below box edge
    ASSERT_NEAR(box.min_distance_to(0.0, -4.0), 4.0, 1e-9);
}

// --- BoundingBox::subdivide ---

TEST(BoundingBox, SubdivideProducesQuadrants) {
    BoundingBox box(0.0, 0.0, 10.0, 10.0);
    auto [sw, se, nw, ne] = box.subdivide();

    ASSERT_NEAR(sw.half_w, 5.0, 1e-9);
    ASSERT_NEAR(sw.half_h, 5.0, 1e-9);

    // SW center should be (-5, -5)
    ASSERT_NEAR(sw.cx, -5.0, 1e-9);
    ASSERT_NEAR(sw.cy, -5.0, 1e-9);

    // NE center should be (5, 5)
    ASSERT_NEAR(ne.cx, 5.0, 1e-9);
    ASSERT_NEAR(ne.cy, 5.0, 1e-9);
}

// --- QuadTree insert + size tracking ---

TEST(QuadTree, InsertAndSize) {
    BoundingBox bounds(5.0, 5.0, 5.0, 5.0);  // [0,10] x [0,10]
    QuadTree tree(bounds);

    ASSERT_EQ(tree.size(), 0);

    ASSERT_TRUE(tree.insert(Point(1.0, 1.0)));
    ASSERT_EQ(tree.size(), 1);

    ASSERT_TRUE(tree.insert(Point(3.0, 3.0)));
    ASSERT_TRUE(tree.insert(Point(7.0, 7.0)));
    ASSERT_EQ(tree.size(), 3);
}

TEST(QuadTree, InsertOutsideBoundaryFails) {
    BoundingBox bounds(5.0, 5.0, 5.0, 5.0);
    QuadTree tree(bounds);

    ASSERT_FALSE(tree.insert(Point(-1.0, 5.0)));
    ASSERT_FALSE(tree.insert(Point(5.0, 11.0)));
    ASSERT_FALSE(tree.insert(Point(100.0, 100.0)));
    ASSERT_EQ(tree.size(), 0);
}

TEST(QuadTree, InsertTriggersSubdivision) {
    BoundingBox bounds(5.0, 5.0, 5.0, 5.0);
    QuadTree tree(bounds, 2);  // capacity 2 — subdivides after 2 points

    tree.insert(Point(1.0, 1.0));
    tree.insert(Point(2.0, 2.0));
    ASSERT_FALSE(tree.divided);

    tree.insert(Point(3.0, 3.0));  // triggers subdivision
    ASSERT_TRUE(tree.divided);
    ASSERT_EQ(tree.size(), 3);
}

TEST(QuadTree, InsertOnBoundaryEdge) {
    BoundingBox bounds(5.0, 5.0, 5.0, 5.0);  // [0,10] x [0,10]
    QuadTree tree(bounds);

    ASSERT_TRUE(tree.insert(Point(0.0, 0.0)));   // corner
    ASSERT_TRUE(tree.insert(Point(10.0, 10.0))); // corner
    ASSERT_TRUE(tree.insert(Point(5.0, 0.0)));   // edge
    ASSERT_EQ(tree.size(), 3);
}

TEST(QuadTree, InsertManyPointsSizeTracking) {
    BoundingBox bounds(50.0, 50.0, 50.0, 50.0);  // [0,100] x [0,100]
    QuadTree tree(bounds, 4);

    for (int i = 1; i <= 50; ++i) {
        double v = static_cast<double>(i);
        ASSERT_TRUE(tree.insert(Point(v, v)));
    }
    ASSERT_EQ(tree.size(), 50);
}
