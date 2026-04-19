// ParkingSpot and AvailabilityTracker tests

#include "test_framework.h"
#include "../src/models/parking_spot.h"
#include "../src/models/availability_tracker.h"

// --- ParkingSpot tests ---

TEST(ParkingSpot, Creation) {
    ParkingSpot spot("S1", {3.0, 4.0}, "zone-A", "compact");
    ASSERT_EQ(spot.spot_id, "S1");
    ASSERT_NEAR(spot.x(), 3.0, 0.001);
    ASSERT_NEAR(spot.y(), 4.0, 0.001);
    ASSERT_EQ(spot.zone_id, "zone-A");
    ASSERT_EQ(spot.spot_type, "compact");
    ASSERT_TRUE(spot.is_available());
}

TEST(ParkingSpot, DefaultsToAvailableStandard) {
    ParkingSpot spot("S1", {0.0, 0.0}, "zone-A");
    ASSERT_EQ(spot.spot_type, "standard");
    ASSERT_TRUE(spot.is_available());
}

TEST(ParkingSpot, Occupy) {
    ParkingSpot spot("S1", {0.0, 0.0}, "zone-A");
    spot.occupy();
    ASSERT_FALSE(spot.is_available());
    ASSERT_EQ(spot.status, SpotStatus::OCCUPIED);
}

TEST(ParkingSpot, Release) {
    ParkingSpot spot("S1", {0.0, 0.0}, "zone-A");
    spot.occupy();
    spot.release();
    ASSERT_TRUE(spot.is_available());
    ASSERT_EQ(spot.status, SpotStatus::AVAILABLE);
}

TEST(ParkingSpot, Reserve) {
    ParkingSpot spot("S1", {0.0, 0.0}, "zone-A");
    spot.reserve();
    ASSERT_FALSE(spot.is_available());
    ASSERT_EQ(spot.status, SpotStatus::RESERVED);
}

TEST(ParkingSpot, Maintenance) {
    ParkingSpot spot("S1", {0.0, 0.0}, "zone-A");
    spot.set_maintenance();
    ASSERT_FALSE(spot.is_available());
    ASSERT_EQ(spot.status, SpotStatus::MAINTENANCE);
}

TEST(ParkingSpot, StatusHistory) {
    ParkingSpot spot("S1", {0.0, 0.0}, "zone-A");
    spot.occupy();
    spot.release();
    spot.reserve();
    auto history = spot.get_status_history();
    ASSERT_EQ(history.size(), 3);
    ASSERT_EQ(history[0].first, SpotStatus::OCCUPIED);
    ASSERT_EQ(history[1].first, SpotStatus::AVAILABLE);
    ASSERT_EQ(history[2].first, SpotStatus::RESERVED);
}

TEST(ParkingSpot, SameStatusNoOp) {
    ParkingSpot spot("S1", {0.0, 0.0}, "zone-A");
    spot.occupy();
    spot.occupy(); // Same status — should not add to history
    auto history = spot.get_status_history();
    ASSERT_EQ(history.size(), 1);
}

TEST(ParkingSpot, CallbackFires) {
    ParkingSpot spot("S1", {0.0, 0.0}, "zone-A");
    bool called = false;
    std::string cb_id;
    SpotStatus cb_old, cb_new;
    spot.register_status_callback(
        [&](const std::string& id, SpotStatus old_s, SpotStatus new_s) {
            called = true; cb_id = id; cb_old = old_s; cb_new = new_s;
        });
    spot.occupy();
    ASSERT_TRUE(called);
    ASSERT_EQ(cb_id, "S1");
    ASSERT_EQ(cb_old, SpotStatus::AVAILABLE);
    ASSERT_EQ(cb_new, SpotStatus::OCCUPIED);
}

TEST(ParkingSpot, StatusToString) {
    ASSERT_EQ(spot_status_to_string(SpotStatus::AVAILABLE), "available");
    ASSERT_EQ(spot_status_to_string(SpotStatus::OCCUPIED), "occupied");
    ASSERT_EQ(spot_status_to_string(SpotStatus::RESERVED), "reserved");
    ASSERT_EQ(spot_status_to_string(SpotStatus::MAINTENANCE), "maintenance");
}

TEST(ParkingSpot, StringToStatus) {
    ASSERT_EQ(string_to_spot_status("available"), SpotStatus::AVAILABLE);
    ASSERT_EQ(string_to_spot_status("occupied"), SpotStatus::OCCUPIED);
    ASSERT_EQ(string_to_spot_status("garbage"), SpotStatus::UNKNOWN);
}

// --- AvailabilityTracker tests ---

TEST(Tracker, RegisterAndCount) {
    AvailabilityTracker tracker;
    tracker.register_spot("S1", "zone-A", true);
    tracker.register_spot("S2", "zone-A", true);
    tracker.register_spot("S3", "zone-B", true);
    ASSERT_EQ(tracker.count_available(), 3);
    ASSERT_EQ(tracker.count_available("zone-A"), 2);
    ASSERT_EQ(tracker.count_available("zone-B"), 1);
}

TEST(Tracker, IsAvailable) {
    AvailabilityTracker tracker;
    tracker.register_spot("S1", "zone-A", true);
    tracker.register_spot("S2", "zone-A", false);
    ASSERT_TRUE(tracker.is_available("S1"));
    ASSERT_FALSE(tracker.is_available("S2"));
    ASSERT_FALSE(tracker.is_available("S99")); // Not registered
}

TEST(Tracker, OnStatusChange) {
    AvailabilityTracker tracker;
    tracker.register_spot("S1", "zone-A", true);
    ASSERT_TRUE(tracker.is_available("S1"));

    tracker.on_status_change("S1", SpotStatus::AVAILABLE, SpotStatus::OCCUPIED);
    ASSERT_FALSE(tracker.is_available("S1"));
    ASSERT_EQ(tracker.count_available(), 0);

    tracker.on_status_change("S1", SpotStatus::OCCUPIED, SpotStatus::AVAILABLE);
    ASSERT_TRUE(tracker.is_available("S1"));
    ASSERT_EQ(tracker.count_available(), 1);
}

TEST(Tracker, ZoneOccupancyRate) {
    AvailabilityTracker tracker;
    tracker.register_spot("S1", "zone-A", true);
    tracker.register_spot("S2", "zone-A", true);
    ASSERT_NEAR(tracker.get_zone_occupancy_rate("zone-A"), 0.0, 0.001);

    tracker.on_status_change("S1", SpotStatus::AVAILABLE, SpotStatus::OCCUPIED);
    ASSERT_NEAR(tracker.get_zone_occupancy_rate("zone-A"), 0.5, 0.001);

    tracker.on_status_change("S2", SpotStatus::AVAILABLE, SpotStatus::OCCUPIED);
    ASSERT_NEAR(tracker.get_zone_occupancy_rate("zone-A"), 1.0, 0.001);
}

TEST(Tracker, GetAvailableInZone) {
    AvailabilityTracker tracker;
    tracker.register_spot("S1", "zone-A", true);
    tracker.register_spot("S2", "zone-A", true);
    tracker.register_spot("S3", "zone-B", true);
    auto zone_a = tracker.get_available_in_zone("zone-A");
    ASSERT_EQ(zone_a.size(), 2);
    ASSERT_TRUE(zone_a.count("S1") > 0);
    ASSERT_TRUE(zone_a.count("S2") > 0);
}

TEST(Tracker, UnregisterSpot) {
    AvailabilityTracker tracker;
    tracker.register_spot("S1", "zone-A", true);
    tracker.register_spot("S2", "zone-A", true);
    ASSERT_EQ(tracker.count_available(), 2);

    tracker.unregister_spot("S1");
    ASSERT_FALSE(tracker.is_available("S1"));
    ASSERT_EQ(tracker.count_available(), 1);
}

TEST(Tracker, CallbackIntegration) {
    ParkingSpot spot("S1", {0.0, 0.0}, "zone-A");
    AvailabilityTracker tracker;
    tracker.register_spot("S1", "zone-A", true);

    spot.register_status_callback(
        [&tracker](const std::string& id, SpotStatus old_s, SpotStatus new_s) {
            tracker.on_status_change(id, old_s, new_s);
        });

    ASSERT_TRUE(tracker.is_available("S1"));
    spot.occupy();
    ASSERT_FALSE(tracker.is_available("S1"));
    ASSERT_NEAR(tracker.get_zone_occupancy_rate("zone-A"), 1.0, 0.001);

    spot.release();
    ASSERT_TRUE(tracker.is_available("S1"));
    ASSERT_NEAR(tracker.get_zone_occupancy_rate("zone-A"), 0.0, 0.001);
}

TEST(Tracker, EmptyZoneOccupancy) {
    AvailabilityTracker tracker;
    ASSERT_NEAR(tracker.get_zone_occupancy_rate("nonexistent"), 0.0, 0.001);
}
