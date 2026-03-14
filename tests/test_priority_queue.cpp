// PriorityQueue tests — basic push/pop ordering

#include "test_framework.h"
#include "../src/data_structures/priority_queue.h"

TEST(PriorityQueue, PopReturnsLowestPriority) {
    PriorityQueue pq;
    pq.push("c", 3.0);
    pq.push("a", 1.0);
    pq.push("b", 2.0);

    ASSERT_EQ(pq.pop().value(), "a");
    ASSERT_EQ(pq.pop().value(), "b");
    ASSERT_EQ(pq.pop().value(), "c");
}

TEST(PriorityQueue, SizeAndEmpty) {
    PriorityQueue pq;
    ASSERT_TRUE(pq.empty());
    ASSERT_EQ(pq.size(), 0);

    pq.push("x", 5.0);
    ASSERT_FALSE(pq.empty());
    ASSERT_EQ(pq.size(), 1);

    pq.push("y", 3.0);
    ASSERT_EQ(pq.size(), 2);

    pq.pop();
    ASSERT_EQ(pq.size(), 1);

    pq.pop();
    ASSERT_TRUE(pq.empty());
}

TEST(PriorityQueue, PeekDoesNotRemove) {
    PriorityQueue pq;
    pq.push("a", 1.0);
    pq.push("b", 2.0);

    ASSERT_EQ(pq.peek().value(), "a");
    ASSERT_EQ(pq.size(), 2);
    ASSERT_EQ(pq.peek().value(), "a");
}

TEST(PriorityQueue, PopWithPriority) {
    PriorityQueue pq;
    pq.push("a", 1.5);
    pq.push("b", 2.5);

    auto result = pq.pop_with_priority();
    ASSERT_TRUE(result.has_value());
    ASSERT_EQ(result->first, "a");
    ASSERT_NEAR(result->second, 1.5, 1e-9);
}

TEST(PriorityQueue, PopEmptyReturnsNullopt) {
    PriorityQueue pq;
    ASSERT_FALSE(pq.pop().has_value());
    ASSERT_FALSE(pq.peek().has_value());
    ASSERT_FALSE(pq.peek_priority().has_value());
}

TEST(PriorityQueue, PushUpdatesExistingItem) {
    PriorityQueue pq;
    pq.push("a", 5.0);
    pq.push("b", 3.0);
    pq.push("a", 1.0);  // update a to higher priority

    ASSERT_EQ(pq.pop().value(), "a");
    ASSERT_EQ(pq.pop().value(), "b");
}

TEST(PriorityQueue, FIFOBreaksTies) {
    PriorityQueue pq;
    pq.push("first", 1.0);
    pq.push("second", 1.0);
    pq.push("third", 1.0);

    ASSERT_EQ(pq.pop().value(), "first");
    ASSERT_EQ(pq.pop().value(), "second");
    ASSERT_EQ(pq.pop().value(), "third");
}

// --- Commit 4: decrease_key and contains tests ---

TEST(PriorityQueue, ContainsFindsItems) {
    PriorityQueue pq;
    ASSERT_FALSE(pq.contains("a"));

    pq.push("a", 1.0);
    ASSERT_TRUE(pq.contains("a"));
    ASSERT_FALSE(pq.contains("b"));

    pq.pop();
    ASSERT_FALSE(pq.contains("a"));
}

TEST(PriorityQueue, DecreaseKeyLowersPriority) {
    PriorityQueue pq;
    pq.push("a", 5.0);
    pq.push("b", 3.0);

    // b is currently first (priority 3). Decrease a to 1 so it comes first.
    bool ok = pq.decrease_key("a", 1.0);
    ASSERT_TRUE(ok);

    ASSERT_EQ(pq.pop().value(), "a");
    ASSERT_EQ(pq.pop().value(), "b");
}

TEST(PriorityQueue, DecreaseKeyRejectsHigherPriority) {
    PriorityQueue pq;
    pq.push("a", 2.0);

    bool ok = pq.decrease_key("a", 5.0);  // 5 > 2, should fail
    ASSERT_FALSE(ok);

    ASSERT_NEAR(pq.peek_priority().value(), 2.0, 1e-9);
}

TEST(PriorityQueue, DecreaseKeyNonexistentReturnsFalse) {
    PriorityQueue pq;
    pq.push("a", 1.0);

    ASSERT_FALSE(pq.decrease_key("z", 0.5));
}

TEST(PriorityQueue, GetPriority) {
    PriorityQueue pq;
    pq.push("a", 3.0);
    pq.push("b", 7.0);

    ASSERT_NEAR(pq.get_priority("a").value(), 3.0, 1e-9);
    ASSERT_NEAR(pq.get_priority("b").value(), 7.0, 1e-9);
    ASSERT_FALSE(pq.get_priority("z").has_value());
}
