// author : Mauricio Gomes
// license: MIT (https://opensource.org/licenses/MIT)

#include "../../../unit_test/src/test.hpp"


#include "../statistic.hpp"
#include "../random_util.hpp"

using namespace pensar_digital::cpplib;

namespace pensar_digital
{
    namespace test = pensar_digital::unit_test;
    using namespace pensar_digital::unit_test;
    namespace cpplib
    {
        TEST(RandomUtilTest, true)

            CRandom r(0, 9);
        const int N = 100000;
        int a[N];

        for (int i = 0; i < N; ++i)
        {
            a[i] = r.get();
        }

        // Calculates the standard deviation on a.
        auto sd = pensar_digital::cpplib::standard_deviation(a, a + N);
        CHECK(sd < 3, "Standard deviation is too high.");

        TEST_END(RandomUtilTest)
    }
}