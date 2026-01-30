// author : Mauricio Gomes
// license: MIT (https://opensource.org/licenses/MIT)

#include "../../../unit_test/src/test.hpp"

#include "../wire_double.hpp"
#include "../equal.hpp"

#include <bit>
#include <cmath>

namespace pensar_digital
{
    using namespace pensar_digital::unit_test;
    namespace cpplib
    {
        TEST(Double64Basics, true)
            WireDouble d0;
            CHECK_EQ(uint64_t, d0.bits, 0, W("0. default bits should be zero"));

            WireDouble d1(3.5);
            CHECK(std::abs(d1 - 3.5) < Test::DEFAULT_DELTA, W("1. conversion to double"));

            WireDouble d2(-2.25);
            CHECK(std::abs(d2 + 2.25) < Test::DEFAULT_DELTA, W("2. bit constructor"));

            WireDouble d3(d1 + 1.25);
            CHECK(std::abs(d3 - 4.75) < Test::DEFAULT_DELTA, W("3. operator+"));

            d3 *= WireDouble(2.0);
            CHECK(std::abs(d3 - 9.5) < Test::DEFAULT_DELTA, W("4. operator*="));
        TEST_END(Double64Basics)

        TEST(Double64MemcmpComparable, true)
            WireDouble a(1.0);
            WireDouble b(1.0);
            WireDouble c(2.0);

            CHECK_EQ(bool, equal<WireDouble>(a, b), true, W("0. equal via memcmp"));
            CHECK_EQ(bool, equal<WireDouble>(a, c), false, W("1. not equal via memcmp"));
        TEST_END(Double64MemcmpComparable)
    }
}
