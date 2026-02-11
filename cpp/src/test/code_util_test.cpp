// author : Mauricio Gomes
// license: MIT (https://opensource.org/licenses/MIT)

#include <catch2/catch_test_macros.hpp>

#include "../code_util.hpp"

namespace pensar_digital::cpplib::code_util
{
    using R = Result<int>;

    R f() { return 10; }
    R f1() { return W("err msg"); }

    TEST_CASE("Result", "[code_util]")
    {
        R r = f();
        INFO(W("0. r != 10")); CHECK((r == 10));
        INFO(W("0. r != Bool::T")); CHECK((r.mok == Bool{Bool::T}));

        R r1 = f1();
        INFO("r1 must be false."); CHECK(!r1);
        INFO(W("0. r1.merror_message != err msg")); CHECK((r1.merror_message == R::ErrorMessageType(W("err msg"))));
        INFO(W("1. r1.mok != Bool::F")); CHECK((Bool(r1) == Bool{Bool::F}));
    }

    TEST_CASE("ResultSerialization", "[code_util]")
    {
        using R = Result<int>;
        R r;

        R r1(0, Bool::F, R::ErrorMessageType(W("err msg")));
        INFO(W("0. r == r1")); CHECK(r.mok != r1.mok);
        r1 = r;
        INFO(W("1. r != r1")); CHECK((r == r1));
        INFO(W("1b. r.mok != r1.mok")); CHECK((r.mok == r1.mok));
    }
}
