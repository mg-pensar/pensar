// author : Mauricio Gomes
// license: MIT (https://opensource.org/licenses/MIT)

#include "../../../unit_test/src/test.hpp"

#include "../double64.hpp"
#include "../equal.hpp"

#include <bit>
#include <cmath>

namespace pensar_digital
{
    using namespace pensar_digital::unit_test;
    namespace cpplib
    {
        TEST(Double64Basics, true)
            Double64 d0;
            CHECK_EQ(uint64_t, d0.bits, 0, W("0. default bits should be zero"));

            Double64 d1(3.5);
            CHECK(std::abs(static_cast<double>(d1) - 3.5) < Test::DEFAULT_DELTA, W("1. conversion to double"));

            Double64 d2(std::bit_cast<uint64_t>(-2.25));
            CHECK(std::abs(static_cast<double>(d2) + 2.25) < Test::DEFAULT_DELTA, W("2. bit constructor"));

            Double64 d3 = d1 + 1.25;
            CHECK(std::abs(static_cast<double>(d3) - 4.75) < Test::DEFAULT_DELTA, W("3. operator+"));

            d3 *= 2.0;
            CHECK(std::abs(static_cast<double>(d3) - 9.5) < Test::DEFAULT_DELTA, W("4. operator*="));
        TEST_END(Double64Basics)

        TEST(Double64MemcmpComparable, true)
            Double64 a(1.0);
            Double64 b(1.0);
            Double64 c(2.0);

            CHECK_EQ(bool, equal<Double64>(a, b), true, W("0. equal via memcmp"));
            CHECK_EQ(bool, equal<Double64>(a, c), false, W("1. not equal via memcmp"));
        TEST_END(Double64MemcmpComparable)
    }
}
