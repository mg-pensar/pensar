// author : Mauricio Gomes
// license: MIT (https://opensource.org/licenses/MIT)

#include <catch2/catch_test_macros.hpp>
#include "../e-mail.hpp"

namespace pensar_digital::cpplib::contact
{
    TEST_CASE("Email validation", "[email]")
    {
        SECTION("Valid email addresses")
        {
            CHECK(is_valid_email_address(W("x@x.com")));
            CHECK(is_valid_email_address(W("user@example.com")));
            CHECK(is_valid_email_address(W("user.name@example.com")));
            CHECK(is_valid_email_address(W("user+tag@example.co.uk")));
            CHECK(is_valid_email_address(W("a@b.cd")));
        }

        SECTION("Invalid email addresses")
        {
            CHECK_FALSE(is_valid_email_address(W("")));
            CHECK_FALSE(is_valid_email_address(W("noatsign")));
            CHECK_FALSE(is_valid_email_address(W("@domain.com")));
            CHECK_FALSE(is_valid_email_address(W("user@")));
            CHECK_FALSE(is_valid_email_address(W("user@@domain.com")));
            CHECK_FALSE(is_valid_email_address(W("user@.com")));
            CHECK_FALSE(is_valid_email_address(W("user@domain")));
            CHECK_FALSE(is_valid_email_address(W(".user@domain.com")));
            CHECK_FALSE(is_valid_email_address(W("user.@domain.com")));
            CHECK_FALSE(is_valid_email_address(W("us..er@domain.com")));
        }
    }

    TEST_CASE("Email construction and accessors", "[email]")
    {
        Email email(W("x@x.com"));
        CHECK(email.str() == S(W("x@x.com")));

        Email email2(W("local"), W("domain.com"));
        CHECK(email2.str() == S(W("local@domain.com")));
    }

    TEST_CASE("Email equality", "[email]")
    {
        Email a(W("user@example.com"));
        Email b(W("user@example.com"));
        Email c(W("other@example.com"));

        CHECK(a == b);
        CHECK(a != c);
    }

    TEST_CASE("Email text streaming", "[email]")
    {
        Email email(W("test@mail.com"));
        pd::OutStringStream oss;
        oss << email;
        CHECK(oss.str() == S(W("test@mail.com")));

        pd::InStringStream iss(W("read@stream.com"));
        Email email2(W("x@x.com")); // start with a valid email
        iss >> email2;
        CHECK(email2.str() == S(W("read@stream.com")));
    }
} // namespace pensar_digital::cpplib::contact