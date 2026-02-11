// author : Mauricio Gomes
// license: MIT (https://opensource.org/licenses/MIT)

#include <catch2/catch_test_macros.hpp>

#include "../s.hpp"

namespace pensar_digital::cpplib
{
    TEST_CASE("CS", "[string]")
    {
        static_assert(TriviallyCopyable<CS<10>>, W("S is not compliant with StdLayoutTriviallyCopyable concept."));
        CS<> s;

        CS<> s1;
        INFO(W("0")); CHECK(s == s1);
        s = W("abc");
        INFO(W("1")); CHECK(s != s1);
        INFO(W("2")); CHECK(s.to_string() == W("abc"));
        INFO(W("3")); CHECK(s.length() == 3);
        INFO(W("4")); CHECK(s.size() == CS<>::MAX_SIZE);

        CS<> s2 = W("abc");
        INFO(W("5")); CHECK((s2.to_string() == W("abc")));
        INFO(W("6")); CHECK(s == s2);

        s2 = W("def");
        INFO(W("7")); CHECK(s2.to_string() == W("def"));
        static_assert(pd::OutputStreamable<CS<>>, "S is not OutputStreamable");
        typedef CS<0, 20> WC;
        WC w = W("abc");
        static_assert(pd::OutputStreamable<WC>, "W is not OutputStreamable");

        S str = W("abc");
        CS<> s3 = str;
        INFO(W("9")); CHECK(s3.to_string() == W("abc"));

        CS<> s4 = W("abc");
        CS<> s5 = W("def");
        CS<> s6 = s4 + s5;
        INFO(W("11")); CHECK(s6.to_string() == W("abcdef"));
    }
}
