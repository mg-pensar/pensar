// author : Mauricio Gomes
// license: MIT (https://opensource.org/licenses/MIT)

#include "../../../unit_test/src/test.hpp"


#include "../s.hpp"

namespace pensar_digital
{
	using namespace pensar_digital::unit_test;
    namespace cpplib
    {
        TEST(CS, true)
            static_assert(TriviallyCopyable<CS<10>>, W("S is not compliant with StdLayoutTriviallyCopyable concept."));
            CS<> s;
            // Asserts CS has no padding bytes and can be copied with std::memcpy.
 
            CS<> s1;
            CHECK_EQ(CS<>, s, s1, W("0"));
            s = W("abc");
            CHECK_NOT_EQ(CS<>, s, s1, W("1"));
            CHECK_EQ(CS<>, s, W("abc"), W("2"));
            CHECK_EQ(size_t, s.length(), 3, W("3"));
            CHECK_EQ(size_t, s.size(), CS<>::MAX_SIZE, W("4"));

            CS<> s2 = W("abc");
            CHECK_EQ(CS<>, s2.to_string(), W("abc"), W("5"));
            CHECK(s == s2, W("6"));

            s2 = W("def");
            CHECK_EQ(CS<>, s2, W("def"), W("7"));
            static_assert (pd::OutputStreamable<CS<>>, "S is not OutputStreamable");
            typedef CS<0, 20> WC;
            WC w = W("abc");
            static_assert (pd::OutputStreamable<WC>, "W is not OutputStreamable");

            S str = W("abc");
            CS<> s3 = str;
            CHECK_EQ(CS<>, s3, W("abc"), W("9"));


            CS<> s4 = W("abc");
            CS<> s5 = W("def");
            CS<> s6 = s4 + s5;
            CHECK_EQ(CS<>, s6, W("abcdef"), W("11"));

            TEST_END(CS)
    }
}
