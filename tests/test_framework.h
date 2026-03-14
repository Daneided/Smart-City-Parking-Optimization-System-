// Lightweight test framework — no external dependencies
// Provides TEST, ASSERT_EQ, ASSERT_TRUE macros and a test registry

#ifndef TEST_FRAMEWORK_H
#define TEST_FRAMEWORK_H

#include <functional>
#include <iostream>
#include <string>
#include <vector>

// Global test registry — collects all TEST() instances
struct TestCase {
    std::string suite;
    std::string name;
    std::function<void()> func;
};

inline std::vector<TestCase>& test_registry() {
    static std::vector<TestCase> registry;
    return registry;
}

// Auto-registers a test at static-init time
struct TestRegistrar {
    TestRegistrar(const std::string& suite, const std::string& name,
                  std::function<void()> func) {
        test_registry().push_back({suite, name, std::move(func)});
    }
};

// Thrown on assertion failure to abort the current test
struct AssertionFailure {
    std::string file;
    int line;
    std::string message;
};

// --- Macros ---

#define TEST(suite, name)                                                      \
    void suite##_##name();                                                     \
    static TestRegistrar reg_##suite##_##name(#suite, #name, suite##_##name);  \
    void suite##_##name()

#define ASSERT_TRUE(expr)                                                      \
    do {                                                                        \
        if (!(expr)) {                                                         \
            throw AssertionFailure{__FILE__, __LINE__,                          \
                "ASSERT_TRUE failed: " #expr};                                 \
        }                                                                      \
    } while (0)

#define ASSERT_FALSE(expr)                                                     \
    do {                                                                        \
        if ((expr)) {                                                          \
            throw AssertionFailure{__FILE__, __LINE__,                          \
                "ASSERT_FALSE failed: " #expr};                                \
        }                                                                      \
    } while (0)

#define ASSERT_EQ(a, b)                                                        \
    do {                                                                        \
        if (!((a) == (b))) {                                                   \
            throw AssertionFailure{__FILE__, __LINE__,                          \
                "ASSERT_EQ failed: " #a " == " #b};                            \
        }                                                                      \
    } while (0)

#endif // TEST_FRAMEWORK_H
