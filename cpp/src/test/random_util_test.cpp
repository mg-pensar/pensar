// author : Mauricio Gomes
// license: MIT (https://opensource.org/licenses/MIT)

#include <catch2/catch_test_macros.hpp>

#include "../statistic.hpp"
#include "../random_util.hpp"

using namespace pensar_digital::cpplib;

namespace pensar_digital::cpplib
{
    TEST_CASE("RandomUtilTest", "[random]")
    {
        CRandom r(0, 9);
        const int N = 100000;
        int a[N];

        for (int i = 0; i < N; ++i)
        {
            a[i] = r.get();
        }

        auto sd = pensar_digital::cpplib::standard_deviation(a, a + N);
        INFO("Standard deviation is too high.");
        CHECK(sd < 3);
    }
}
