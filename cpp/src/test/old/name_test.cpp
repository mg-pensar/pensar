// author : Mauricio Gomes
// license: MIT (https://opensource.org/licenses/MIT)

#include "../../unit-test/test.hpp"
#include "../name.hpp"
#include "../string_def.hpp"

//#include "wstring_stream_hack.hpp"

/*class Dummy: public pd::Name
{
    public:
      Dummy(const String& aname = L""): pd::Name(aname){};
};
*/

namespace pensar_digital
{
    namespace unit_test
    {
        using namespace pensar_digital::cpplib;
        class Dummy: public Name
        {
            public:
              Dummy(const String& aname = ""): Name(aname){};
        };
        TEST(GetSet)
            String expected = "aName";
            Dummy n(expected);
            CHECK_EQ (String, n.get_name (), expected, "0. It was supposed to be ""aName"".")
            expected = "another name";
            n.set_name(expected);
            CHECK_EQ (String, n.get_name (), expected, "1. It was supposed to be ""another name"".")

            expected = "";
            Dummy n2;
            CHECK_EQ (String, n2.get_name (), expected, "2. It was supposed to be """".")

        TEST_END(GetSet)

        TEST(EqOperator)

            Dummy n;
            Dummy n2;
            CHECK (n == n2, "0. n was supposed to be equal to n2.")

            n.set_name ("a");
            CHECK (! (n == n2), "0. n was supposed to be different of n2.")
        TEST_END(EqOperator)

        TEST(DiffOperator)
            Dummy n;
            Dummy n2;
            CHECK (!(n != n2), "0. n was supposed to be equal to n2.")

            n.set_name ("a");
            CHECK (n != n2   , "1. n was supposed to be different of n2.")
        TEST_END(DiffOperator)
    }
}

/*
TEST(get_set_test)
{
    std::wstring expected = L"aName";
    Dummy<std::wstring> n(expected);
    BOOST_CHECK_EQUAL (n.get_name (), expected);

    expected = L"another name";
    n.set_name(expected);
    BOOST_CHECK_EQUAL (n.get_name (), expected);

    expected = L"";
    Dummy<> n2;
    BOOST_CHECK_EQUAL (n2.get_name (), expected);
}

BOOST_AUTO_TEST_CASE(weq_operator_test)
{
    Dummy<> n;
    Dummy<> n2;
    BOOST_CHECK (n == n2);

    n.set_name (L"a");
    BOOST_CHECK (! (n == n2));
}

BOOST_AUTO_TEST_CASE(wdiff_operator_test)
{
    Dummy<> n;
    Dummy<> n2;
    BOOST_CHECK (!(n != n2));

    n.set_name (L"a");
    BOOST_CHECK (n != n2);
}

BOOST_AUTO_TEST_SUITE_END()
*/
