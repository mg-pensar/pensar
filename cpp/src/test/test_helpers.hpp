#ifndef TEST_HELPERS_HPP
#define TEST_HELPERS_HPP

// Common test utilities for Catch2-based tests.
// Replaces functionality previously provided by the custom unit_test framework.

#include "../path.hpp"
#include "../sys_user_info.hpp"
#include "../string_def.hpp"

#include <catch2/catch_test_macros.hpp>
#include <cmath>
#include <cstddef>

namespace pensar_digital::cpplib::test_helpers
{
    /// Default delta for floating-point comparisons.
    constexpr double DEFAULT_DELTA = 0.0000001;

    /// Returns the shared test output directory (~/test_dir/).
    /// Mirrors the old Test::test_dir() method.
    inline Path get_test_dir()
    {
        return get_user_home() / "test_dir/";
    }

    /// Element-wise collection equality check using Catch2 macros.
    /// Replaces the old Test::check_equal_collection method.
    template <typename A, typename E>
    void check_equal_collection(const A& actual, const E& expected)
    {
        REQUIRE(actual.size() == expected.size());
        for (size_t i = 0; i < actual.size(); ++i)
        {
            INFO("index " << i);
            CHECK(actual[i] == expected[i]);
        }
    }
} // namespace pensar_digital::cpplib::test_helpers

#endif // TEST_HELPERS_HPP
