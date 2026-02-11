// author : Mauricio Gomes
// license: MIT (https://opensource.org/licenses/MIT)

#include <catch2/catch_test_macros.hpp>
#include "test_helpers.hpp"

#include "../wire_double.hpp"
#include "../equal.hpp"

#include <bit>
#include <cmath>

namespace pensar_digital::cpplib
{
    using namespace test_helpers;

    TEST_CASE("WireDouble1", "[wire_double]")
    {
        WireDouble d0;
        INFO(W("0. default bits should be zero")); CHECK(d0.bits == 0);

        WireDouble d1(3.5);
        INFO(W("1. conversion to double")); CHECK(std::abs(d1 - 3.5) < DEFAULT_DELTA);

        WireDouble d2(-2.25);
        INFO(W("2. bit constructor")); CHECK(std::abs(d2 + 2.25) < DEFAULT_DELTA);

        WireDouble d3(d1 + 1.25);
        INFO(W("3. operator+")); CHECK(std::abs(d3 - 4.75) < DEFAULT_DELTA);

        d3 *= WireDouble(2.0);
        INFO(W("4. operator*=")); CHECK(std::abs(d3 - 9.5) < DEFAULT_DELTA);
    }

    TEST_CASE("WireDoubleMemcpyMemcmp", "[wire_double]")
    {
        WireDouble a(1.0);
        WireDouble b(1.0);
        WireDouble c(2.0);

        INFO(W("0. equal via memcmp")); CHECK(equal<WireDouble>(a, b) == true);
        INFO(W("1. not equal via memcmp")); CHECK(equal<WireDouble>(a, c) == false);
    }
}
