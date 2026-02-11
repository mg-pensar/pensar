// author : Mauricio Gomes
// license: MIT (https://opensource.org/licenses/MIT)

#include <catch2/catch_test_macros.hpp>

#include "../s.hpp"
#include "../object.hpp"
#include "../constraint.hpp"

namespace pensar_digital::cpplib
{
    TEST_CASE("Constraint", "[constraint]")
    {
        StringConstraint sc(ONLY_DIGITS_REGEX);

        INFO(W("0")); CHECK( sc.ok(W("123") ));
        INFO(W("1")); CHECK(!sc.ok(W("123a")));
        INFO(W("2")); CHECK(!sc.ok(W("a123")));
        INFO(W("3")); CHECK( sc.ok(W("")    ));
        INFO(W("4")); CHECK(!sc.ok(W(" ")   ));
        INFO(W("5")); CHECK(!sc.ok(W(" 123")));
        INFO(W("6")); CHECK( sc.ok(W("0")   ));

        StringConstraint sc2(W("^bola$"));
        INFO(W("7")); CHECK( sc2.ok(W("bola") ));
        INFO(W("8")); CHECK(!sc2.ok(W(" bola")));
        INFO(W("9")); CHECK(!sc2.ok(W("")     ));

        CompositeConstraint<StringConstraint, StringConstraint> sc3 = (sc || sc2);
        INFO(W("10")); CHECK( sc3.ok(W("bola") ));
        INFO(W("11")); CHECK( sc3.ok(W("123")  ));
        INFO(W("12")); CHECK(!sc3.ok(W("123bola")));
    }

    TEST_CASE("RangeConstraint", "[constraint]")
    {
        RangeConstraint<int> rc(0, 10);
        INFO(W("0")); CHECK( rc.ok(0  ));
        INFO(W("1")); CHECK( rc.ok(10 ));
        INFO(W("2")); CHECK( rc.ok(5  ));
        INFO(W("3")); CHECK(!rc.ok(-1 ));
        INFO(W("4")); CHECK(!rc.ok(11 ));
        INFO(W("5")); CHECK(!rc.ok(100));

        RangeConstraint<C> rc2(W('A'), W('E'));
        INFO(W("6"));  CHECK( rc2.ok(W('A')));
        INFO(W("7"));  CHECK( rc2.ok(W('E')));
        INFO(W("8"));  CHECK( rc2.ok(W('C')));
        INFO(W("9"));  CHECK(!rc2.ok(W('@')));
        INFO(W("10")); CHECK(!rc2.ok(W('F')));
        INFO(W("11")); CHECK(!rc2.ok(W('a')));
        INFO(W("12")); CHECK(!rc2.ok(W('e')));

        RangeConstraint<int> rc3(10);
        INFO(W("13")); CHECK( rc3.ok(10));
        INFO(W("14")); CHECK(!rc3.ok(9 ));
        INFO(W("15")); CHECK(!rc3.ok(11));
    }

    TEST_CASE("CompositeConstraints", "[constraint]")
    {
        RangeConstraint<int> baby(0, 2);
        RangeConstraint<int> kid (3, 10);
        RangeConstraint<int> teen(11, 19);
        RangeConstraint<int> adult(20, 60);
        RangeConstraint<int> old_(61, 200);

        typedef CompositeConstraint<RangeConstraint<int>, RangeConstraint<int>> CompositeRangeInt;
        CompositeRangeInt babies_or_kids = (baby || kid);
        INFO(W("0")); CHECK( babies_or_kids.ok( 0));
        INFO(W("1")); CHECK(!babies_or_kids.ok(11));
        INFO(W("2")); CHECK( babies_or_kids.ok( 3));
        INFO(W("3")); CHECK( babies_or_kids.ok(10));

        CompositeRangeInt teens_or_adults = (teen || adult);
        INFO(W("4")); CHECK( teens_or_adults.ok( 11));
        INFO(W("5")); CHECK( teens_or_adults.ok( 19));
        INFO(W("6")); CHECK( teens_or_adults.ok( 20));
        INFO(W("7")); CHECK( teens_or_adults.ok( 60));
        INFO(W("8")); CHECK(!teens_or_adults.ok( 10));
        INFO(W("9")); CHECK(!teens_or_adults.ok( 61));

        typedef CompositeConstraint<CompositeRangeInt, RangeConstraint<int>> Composite3RangeInt;
        Composite3RangeInt no_adults = (babies_or_kids || teen);
        INFO(W("10")); CHECK( no_adults.ok( 0));
        INFO(W("11")); CHECK( no_adults.ok( 3));
        INFO(W("12")); CHECK( no_adults.ok(10));
        INFO(W("13")); CHECK( no_adults.ok(11));
        INFO(W("14")); CHECK( no_adults.ok(19));
        INFO(W("15")); CHECK(!no_adults.ok(20));
        INFO(W("16")); CHECK(!no_adults.ok(60));
        INFO(W("17")); CHECK(!no_adults.ok(61));

        RangeConstraint<int> legal(18, 200);
        INFO(W("10")); CHECK(!legal.ok(0));
        INFO(W("11")); CHECK(!legal.ok(3));
        INFO(W("12")); CHECK(!legal.ok(10));
        INFO(W("13")); CHECK(!legal.ok(11));
        INFO(W("14")); CHECK(legal.ok(19));
        INFO(W("15")); CHECK(legal.ok(20));
        INFO(W("16")); CHECK(legal.ok(60));
        INFO(W("17")); CHECK(legal.ok(61));
    }
}
