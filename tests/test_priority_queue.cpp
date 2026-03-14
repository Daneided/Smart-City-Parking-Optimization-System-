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
