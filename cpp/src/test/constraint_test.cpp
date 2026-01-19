// author : Mauricio Gomes
// license: MIT (https://opensource.org/licenses/MIT)

#include "../../../unit_test/src/test.hpp"

#include "../s.hpp"
#include "../object.hpp"
#include "../constraint.hpp"

namespace pensar_digital
{
    namespace test = pensar_digital::unit_test;
    using namespace pensar_digital::unit_test;
    namespace cpplib
    {
        TEST(Constraint, true)
            StringConstraint sc (ONLY_DIGITS_REGEX);

            CHECK( sc.ok (W("123") ), W("0"));
            CHECK(!sc.ok (W("123a")), W("1"));
            CHECK(!sc.ok (W("a123")), W("2"));
            CHECK( sc.ok (W("")    ), W("3"));
            CHECK(!sc.ok (W(" ")   ), W("4"));
            CHECK(!sc.ok (W(" 123")), W("5"));
            CHECK( sc.ok (W("0")   ), W("6"));

            StringConstraint sc2(W("^bola$"));
            CHECK( sc2.ok (W("bola") ), W("7"));
            CHECK(!sc2.ok (W(" bola")), W("8"));
            CHECK(!sc2.ok (W("")     ), W("9"));

            CompositeConstraint<StringConstraint, StringConstraint> sc3 = (sc || sc2);
            CHECK( sc3.ok (W("bola") ), W("10"));
            CHECK( sc3.ok (W("123")  ), W("11"));
            CHECK(!sc3.ok (W("123bola")), W("12"));
        TEST_END (Constraint);

        TEST(RangeConstraint, true)
			RangeConstraint<int> rc (0, 10);
			CHECK( rc.ok (0  ), W("0"));
			CHECK( rc.ok (10 ), W("1"));
			CHECK( rc.ok (5  ), W("2"));
			CHECK(!rc.ok (-1 ), W("3"));
			CHECK(!rc.ok (11 ), W("4"));
            CHECK(!rc.ok (100), W("5"));

            RangeConstraint<C> rc2 (W('A'), W('E'));
            CHECK( rc2.ok (W('A')), W("6"));
            CHECK( rc2.ok (W('E')), W("7"));
            CHECK( rc2.ok (W('C')), W("8"));
            CHECK(!rc2.ok (W('@')), W("9"));
            CHECK(!rc2.ok (W('F')), W("10"));
            CHECK(!rc2.ok (W('a')), W("11"));
            CHECK(!rc2.ok (W('e')), W("12"));

            RangeConstraint<int> rc3 (10);
            CHECK( rc3.ok (10), W("13"));
            CHECK(!rc3.ok (9 ), W("14"));
            CHECK(!rc3.ok (11), W("15"));
		TEST_END (RangeConstraint);

        TEST(CompositeConstraints, true)
            RangeConstraint<int> baby (0, 2);
            RangeConstraint<int> kid  (3, 10);
            RangeConstraint<int> teen (11, 19);
            RangeConstraint<int> adult(20, 60);
            RangeConstraint<int> old  (61, 200);

            typedef CompositeConstraint<RangeConstraint<int>, RangeConstraint<int>> CompositeRangeInt;
            CompositeRangeInt babies_or_kids = (baby || kid);
			CHECK( babies_or_kids.ok ( 0), W("0"));
			CHECK(!babies_or_kids.ok (11), W("1"));
            CHECK( babies_or_kids.ok ( 3), W("2"));
            CHECK( babies_or_kids.ok (10), W("3"));

            CompositeRangeInt teens_or_adults = (teen || adult);
            CHECK( teens_or_adults.ok ( 11), W("4"));
            CHECK( teens_or_adults.ok ( 19), W("5"));
            CHECK( teens_or_adults.ok ( 20), W("6"));
            CHECK( teens_or_adults.ok ( 60), W("7"));
            CHECK(!teens_or_adults.ok ( 10), W("8"));
            CHECK(!teens_or_adults.ok ( 61), W("9"));

            typedef CompositeConstraint<CompositeRangeInt, RangeConstraint<int>> Composite3RangeInt;
            Composite3RangeInt no_adults = (babies_or_kids || teen);
            CHECK( no_adults.ok ( 0), W("10"));
            CHECK( no_adults.ok ( 3), W("11"));
            CHECK( no_adults.ok ( 10), W("12"));
            CHECK( no_adults.ok ( 11), W("13"));
            CHECK( no_adults.ok ( 19), W("14"));
            CHECK(!no_adults.ok ( 20), W("15"));
            CHECK(!no_adults.ok ( 60), W("16"));
            CHECK(!no_adults.ok ( 61), W("17"));

            RangeConstraint<int> legal (18, 200);
            CHECK(!legal.ok(0), W("10"));
            CHECK(!legal.ok(3), W("11"));
            CHECK(!legal.ok(10), W("12"));
            CHECK(!legal.ok(11), W("13"));
            CHECK(legal.ok(19), W("14"));
            CHECK(legal.ok(20), W("15"));
            CHECK(legal.ok(60), W("16"));
            CHECK(legal.ok(61), W("17"));

        TEST_END (CompositeConstraints)
    }   // cpplib.
}   // pensar_digital.
