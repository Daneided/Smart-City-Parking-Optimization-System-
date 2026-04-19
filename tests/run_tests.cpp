// Test runner — compiles and runs all test suites

#include "test_framework.h"

// Test files are compiled together; their TEST() macros auto-register
// via static initialization. This file just provides main().

int main() {
    std::cout << "\n=== Smart City Parking — Test Suite ===" << std::endl;
    std::cout << std::endl;
    int failures = run_all_tests();
    std::cout << std::endl;
    return failures;
}
