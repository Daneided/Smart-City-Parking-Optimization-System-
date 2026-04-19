// SpotSearcher tests — spatial search with availability filtering

#include "test_framework.h"
#include "../src/algorithms/search.h"
#include "../src/models/availability_tracker.h"

// Helper: build a QuadTree with parking spot points
static SpotData make_spot_data(const std::string& spot_id, const std::string& zone_id,
                               const std::string& spot_type = "standard") {
    return SpotData{spot_id, zone_id, spot_type};
}

TEST(SpotSearcher, SearchNearestFindsClosest) {
    QuadTree tree(BoundingBox(0, 0, 100, 100));
    tree.insert(Point(10, 10, make_spot_data("S1", "zone-A")));
    tree.insert(Point(50, 50, make_spot_data("S2", "zone-B")));
    tree.insert(Point(90, 90, make_spot_data("S3", "zone-B")));

    // All spots available
    auto checker = [](const std::string&) { return true; };
    SpotSearcher searcher(&tree, checker);

    auto result = searcher.search_nearest({12.0, 12.0});
    ASSERT_TRUE(result.has_value());
    ASSERT_EQ(result->spot_id, "S1");
}

TEST(SpotSearcher, SearchNearestSkipsUnavailable) {
    QuadTree tree(BoundingBox(0, 0, 100, 100));
    tree.insert(Point(10, 10, make_spot_data("S1", "zone-A")));
    tree.insert(Point(20, 20, make_spot_data("S2", "zone-A")));

    // S1 is occupied, only S2 available
    auto checker = [](const std::string& id) { return id != "S1"; };
    SpotSearcher searcher(&tree, checker);

    auto result = searcher.search_nearest({10.0, 10.0});
    ASSERT_TRUE(result.has_value());
    ASSERT_EQ(result->spot_id, "S2");
}

TEST(SpotSearcher, SearchNearestReturnsNulloptWhenAllOccupied) {
    QuadTree tree(BoundingBox(0, 0, 100, 100));
    tree.insert(Point(10, 10, make_spot_data("S1", "zone-A")));
    tree.insert(Point(20, 20, make_spot_data("S2", "zone-A")));

    auto checker = [](const std::string&) { return false; };
    SpotSearcher searcher(&tree, checker);

    auto result = searcher.search_nearest({10.0, 10.0});
    ASSERT_FALSE(result.has_value());
}

TEST(SpotSearcher, SearchNearestEmptyTree) {
    QuadTree tree(BoundingBox(0, 0, 100, 100));
    auto checker = [](const std::string&) { return true; };
    SpotSearcher searcher(&tree, checker);

    auto result = searcher.search_nearest({10.0, 10.0});
    ASSERT_FALSE(result.has_value());
}

TEST(SpotSearcher, SearchNearestNullIndex) {
    SpotSearcher searcher(nullptr, nullptr);
    auto result = searcher.search_nearest({10.0, 10.0});
    ASSERT_FALSE(result.has_value());
}

TEST(SpotSearcher, SearchNearestFiltersByType) {
    QuadTree tree(BoundingBox(0, 0, 100, 100));
    tree.insert(Point(10, 10, make_spot_data("S1", "zone-A", "compact")));
    tree.insert(Point(15, 15, make_spot_data("S2", "zone-A", "handicapped")));
    tree.insert(Point(80, 80, make_spot_data("S3", "zone-B", "handicapped")));

    auto checker = [](const std::string&) { return true; };
    SpotSearcher searcher(&tree, checker);

    // Want handicapped — should skip S1 (closest but compact), return S2
    auto result = searcher.search_nearest({10.0, 10.0}, "handicapped");
    ASSERT_TRUE(result.has_value());
    ASSERT_EQ(result->spot_id, "S2");
}

TEST(SpotSearcher, SearchReturnsMultipleResults) {
    QuadTree tree(BoundingBox(0, 0, 100, 100));
    tree.insert(Point(10, 10, make_spot_data("S1", "zone-A")));
    tree.insert(Point(20, 20, make_spot_data("S2", "zone-A")));
    tree.insert(Point(30, 30, make_spot_data("S3", "zone-B")));

    auto checker = [](const std::string&) { return true; };
    SpotSearcher searcher(&tree, checker);

    SearchCriteria criteria;
    criteria.location = {0.0, 0.0};
    criteria.max_results = 3;
    auto results = searcher.search(criteria);
    ASSERT_EQ(results.size(), 3);
    // Should be sorted by distance (closest first)
    ASSERT_EQ(results[0].spot_id, "S1");
    ASSERT_EQ(results[1].spot_id, "S2");
    ASSERT_EQ(results[2].spot_id, "S3");
}

TEST(SpotSearcher, SearchRespectsMaxResults) {
    QuadTree tree(BoundingBox(0, 0, 100, 100));
    tree.insert(Point(10, 10, make_spot_data("S1", "zone-A")));
    tree.insert(Point(20, 20, make_spot_data("S2", "zone-A")));
    tree.insert(Point(30, 30, make_spot_data("S3", "zone-B")));

    auto checker = [](const std::string&) { return true; };
    SpotSearcher searcher(&tree, checker);

    SearchCriteria criteria;
    criteria.location = {0.0, 0.0};
    criteria.max_results = 2;
    auto results = searcher.search(criteria);
    ASSERT_EQ(results.size(), 2);
}

TEST(SpotSearcher, SearchFiltersUnavailable) {
    QuadTree tree(BoundingBox(0, 0, 100, 100));
    tree.insert(Point(10, 10, make_spot_data("S1", "zone-A")));
    tree.insert(Point(20, 20, make_spot_data("S2", "zone-A")));
    tree.insert(Point(30, 30, make_spot_data("S3", "zone-B")));

    // Only S2 and S3 available
    auto checker = [](const std::string& id) { return id != "S1"; };
    SpotSearcher searcher(&tree, checker);

    SearchCriteria criteria;
    criteria.location = {0.0, 0.0};
    criteria.max_results = 10;
    auto results = searcher.search(criteria);
    ASSERT_EQ(results.size(), 2);
    ASSERT_EQ(results[0].spot_id, "S2");
    ASSERT_EQ(results[1].spot_id, "S3");
}

TEST(SpotSearcher, SearchFiltersBySpotType) {
    QuadTree tree(BoundingBox(0, 0, 100, 100));
    tree.insert(Point(10, 10, make_spot_data("S1", "zone-A", "compact")));
    tree.insert(Point(20, 20, make_spot_data("S2", "zone-A", "standard")));
    tree.insert(Point(30, 30, make_spot_data("S3", "zone-B", "compact")));

    auto checker = [](const std::string&) { return true; };
    SpotSearcher searcher(&tree, checker);

    SearchCriteria criteria;
    criteria.location = {0.0, 0.0};
    criteria.spot_types = std::vector<std::string>{"compact"};
    criteria.max_results = 10;
    auto results = searcher.search(criteria);
    ASSERT_EQ(results.size(), 2);
    ASSERT_EQ(results[0].spot_id, "S1");
    ASSERT_EQ(results[1].spot_id, "S3");
}

TEST(SpotSearcher, SearchWithMaxDistance) {
    QuadTree tree(BoundingBox(0, 0, 100, 100));
    tree.insert(Point(5, 5, make_spot_data("S1", "zone-A")));
    tree.insert(Point(50, 50, make_spot_data("S2", "zone-B")));
    tree.insert(Point(90, 90, make_spot_data("S3", "zone-C")));

    auto checker = [](const std::string&) { return true; };
    SpotSearcher searcher(&tree, checker);

    SearchCriteria criteria;
    criteria.location = {0.0, 0.0};
    criteria.max_distance = 20.0;
    criteria.max_results = 10;
    auto results = searcher.search(criteria);
    // Only S1 is within 20 units of origin
    ASSERT_EQ(results.size(), 1);
    ASSERT_EQ(results[0].spot_id, "S1");
}

TEST(SpotSearcher, SearchNullIndexReturnsEmpty) {
    SpotSearcher searcher(nullptr, nullptr);
    SearchCriteria criteria;
    criteria.location = {0.0, 0.0};
    auto results = searcher.search(criteria);
    ASSERT_EQ(results.size(), 0);
}

TEST(SpotSearcher, SearchWithAvailabilityTracker) {
    QuadTree tree(BoundingBox(0, 0, 100, 100));
    tree.insert(Point(10, 10, make_spot_data("S1", "zone-A")));
    tree.insert(Point(20, 20, make_spot_data("S2", "zone-A")));

    AvailabilityTracker tracker;
    tracker.register_spot("S1", "zone-A", true);
    tracker.register_spot("S2", "zone-A", true);

    // Mark S1 as occupied via tracker
    tracker.on_status_change("S1", SpotStatus::AVAILABLE, SpotStatus::OCCUPIED);

    auto checker = [&tracker](const std::string& id) {
        return tracker.is_available(id);
    };
    SpotSearcher searcher(&tree, checker);

    auto result = searcher.search_nearest({10.0, 10.0});
    ASSERT_TRUE(result.has_value());
    ASSERT_EQ(result->spot_id, "S2");
}
