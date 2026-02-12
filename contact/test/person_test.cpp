// author : Mauricio Gomes
// license: MIT (https://opensource.org/licenses/MIT)

#include <catch2/catch_test_macros.hpp>
#include "../person.hpp"
#include "../../cpp/src/concept.hpp"
#include "../../cpp/src/binary_buffer.hpp"
#include "../../cpp/src/test/test_helpers.hpp"

namespace pensar_digital::cpplib::contact
{
    using namespace test_helpers;

    // ===== Static assertions =====

    TEST_CASE("Person type traits", "[person]")
    {
        // PersonName
        static_assert(StandardLayout<PersonName>,             "PersonName must be standard layout");
        static_assert(TriviallyCopyable<PersonName>,          "PersonName must be trivially copyable");
        static_assert(StdLayoutTriviallyCopyable<PersonName>, "PersonName must be StdLayoutTriviallyCopyable");

        // PhoneNumber
        static_assert(StandardLayout<PhoneNumber>,             "PhoneNumber must be standard layout");
        static_assert(TriviallyCopyable<PhoneNumber>,          "PhoneNumber must be trivially copyable");
        static_assert(StdLayoutTriviallyCopyable<PhoneNumber>, "PhoneNumber must be StdLayoutTriviallyCopyable");

        // Email
        static_assert(StdLayoutTriviallyCopyable<Email::LocalPart>, "Email::LocalPart must be StdLayoutTriviallyCopyable");
        static_assert(StdLayoutTriviallyCopyable<Email::Domain>,    "Email::Domain must be StdLayoutTriviallyCopyable");
        static_assert(StandardLayout<Email>,             "Email must be standard layout");
        static_assert(TriviallyCopyable<Email>,          "Email must be trivially copyable");
        static_assert(StdLayoutTriviallyCopyable<Email>, "Email must be StdLayoutTriviallyCopyable");

        // Date
        static_assert(StandardLayout<Date>,             "Date must be standard layout");
        static_assert(TriviallyCopyable<Date>,          "Date must be trivially copyable");
        static_assert(StdLayoutTriviallyCopyable<Date>, "Date must be StdLayoutTriviallyCopyable");

        // Person::DataType
        static_assert(StandardLayout<Person::DataType>,             "Person::DataType must be standard layout");
        static_assert(TriviallyCopyable<Person::DataType>,          "Person::DataType must be trivially copyable");
        static_assert(StdLayoutTriviallyCopyable<Person::DataType>, "Person::DataType must be StdLayoutTriviallyCopyable");
        static_assert(NoPadding<Person::DataType>,                  "Person::DataType must have no padding");

        // Person higher-level concepts
        static_assert(Identifiable<Person>,         "Person must be Identifiable");
        static_assert(Hashable<Person>,             "Person must be Hashable");
        static_assert(TriviallyPersistable<Person>, "Person must be TriviallyPersistable");
        static_assert(HasStdLayoutTriviallyCopyableData<Person>, "Person must satisfy HasStdLayoutTriviallyCopyableData");

        SUCCEED("All static_asserts passed");
    }

    // ===== PersonName =====

    TEST_CASE("PersonName construction and accessors", "[person]")
    {
        PersonName name = { W("Mauricio"), W(""), W("Gomes") };

        CHECK(name.name().str() == S(W("Mauricio Gomes")));
        CHECK(name.mfirst.str()  == S(W("Mauricio")));
        CHECK(name.mmiddle.empty());
        CHECK(name.mlast.str()   == S(W("Gomes")));

        PersonName name2 = { W("First"), W("Middle"), W("Last") };
        CHECK_FALSE(name == name2);
    }

    // ===== PhoneNumber =====

    TEST_CASE("PhoneNumber construction and equality", "[person]")
    {
        PhoneNumber phone = { W("55"), W("11"), W("1234567890"), ContactQualifier::Business };

        CHECK(phone.mcountry_code.str() == S(W("55")));
        CHECK(phone.mareacode.str()     == S(W("11")));
        CHECK(phone.mnumber.str()       == S(W("1234567890")));
        CHECK(phone.mqualifier == ContactQualifier::Business);

        PhoneNumber phone2 = { W("55"), W("11"), W("1234567890"), ContactQualifier::Business };
        CHECK(phone == phone2);

        PhoneNumber phone3 = { W("1"), W("800"), W("5551234"), ContactQualifier::Business };
        CHECK(phone != phone3);
    }

    // ===== Date =====

    TEST_CASE("Date construction and equality", "[person]")
    {
        Date d1(1980, 1, 1);
        Date d2(1980, 1, 1);
        CHECK(d1 == d2);

        Date d3(1990, 12, 31);
        CHECK(d1 != d3);
    }

    // ===== Person construction =====

    TEST_CASE("Person default construction", "[person]")
    {
        Person p;
        // Default-constructed person has null name
        CHECK(p.name() == null_person_name());
    }

    TEST_CASE("Person construction with data", "[person]")
    {
        PersonName name = { W("Mauricio"), W(""), W("Gomes") };
        Date dob(1980, 1, 1);
        Person::DataType data = { name, dob };
        Person p(data);

        PersonName expected = { W("Mauricio"), W(""), W("Gomes") };
        CHECK(p.name() == expected);
    }

    // ===== Person accessors =====

    TEST_CASE("Person phone and email accessors", "[person]")
    {
        Person p;
        p.set_phone1({ W("55"), W("11"), W("1234567890"), ContactQualifier::Business });
        p.set_phone2({ W("55"), W("21"), W("9876543210"), ContactQualifier::Personal });
        p.set_email1(Email(W("user@example.com")));
        p.set_email2(Email(W("other@test.org")));

        CHECK(S(p.phone1().mcountry_code) == W("55"));
        CHECK(S(p.phone1().mareacode)     == W("11"));
        CHECK(p.phone2().mareacode.str()     == S(W("21")));
        CHECK(p.email1().str() == S(W("user@example.com")));
        CHECK(p.email2().str() == S(W("other@test.org")));
    }

    // ===== Person equality =====

    TEST_CASE("Person equality", "[person]")
    {
        PersonName name = { W("A"), W(""), W("B") };
        Date dob(2000, 6, 15);
        Person::DataType d = { name, dob };
        Person p1(d, 1);
        Person p2(d, 1);
        CHECK(p1 == p2);

        Person p3;
        CHECK(p1 != p3);
    }

    // ===== BinaryBuffer serialization =====

    TEST_CASE("Person BinaryBuffer round-trip", "[person]")
    {
        PersonName name = { W("Mauricio"), W(""), W("Gomes") };
        Date dob(1980, 1, 1);
        Person::DataType data = { name, dob };
        Person p(data);
        p.set_phone1({ W("55"), W("11"), W("1234567890"), ContactQualifier::Business });
        p.set_phone2({ W("55"), W("21"), W("9876543210"), ContactQualifier::Personal });
        p.set_phone3({ W("1"),  W("800"), W("5551234"),   ContactQualifier::Business });
        p.set_phone4({ W("44"), W("20"),  W("79460000"),  ContactQualifier::Personal });
        p.set_phone5({ W("91"), W("22"),  W("12345678"),  ContactQualifier::Business });
        p.set_email1(Email(W("local1@domain1.com")));
        p.set_email2(Email(W("local2@domain2.com")));
        p.set_email3(Email(W("local3@domain3.com")));
        p.set_email4(Email(W("local4@domain4.com")));
        p.set_email5(Email(W("local5@domain5.com")));

        BinaryBuffer bb;
        bb.write(p);

        Person p2;
        INFO("Before read, persons should differ");
        CHECK(p2 != p);

        bb.read(p2);
        INFO("After read, persons should be equal");
        CHECK(p2 == p);
        CHECK(p2.name() == p.name());
        CHECK(p2.phone1() == p.phone1());
        CHECK(p2.email1() == p.email1());
    }

    TEST_CASE("Person BinaryBuffer file round-trip", "[person]")
    {
        Path test_dir = get_test_dir() / W("PersonFileRoundTrip/");
        test_dir.create_dir();
        Path out = test_dir / Path(W("person_binary.bin"));

        PersonName name = { W("Test"), W("M"), W("User") };
        Date dob(1990, 6, 15);
        Person::DataType data = { name, dob };
        Person p(data);
        p.set_phone1({ W("1"), W("555"), W("1234567"), ContactQualifier::Personal });
        p.set_email1(Email(W("test@user.com")));

        BinaryBuffer bb;
        bb.write(p);
        bb.save_to_file(out.s());

        Person p2;
        CHECK(p != p2);

        BinaryBuffer bb2;
        bb2.load_from_file(out.s());
        bb2.read(p2);
        CHECK(p == p2);
    }

    TEST_CASE("Person BinaryBuffer batch serialization", "[person]")
    {
        const size_t N = 100;

        PersonName name1 = { W("First"), W("Middle"), W("Last") };
        Date dob1(1980, 1, 1);
        Person::DataType d1 = { name1, dob1 };
        Person p1(d1);
        p1.set_phone1({ W("55"), W("11"), W("1234567890"), ContactQualifier::Business });
        p1.set_email1(Email(W("first@example.com")));

        PersonName name2 = { W("Second"), W(""), W("Person") };
        Date dob2(1981, 2, 2);
        Person::DataType d2 = { name2, dob2 };
        Person p2(d2);
        p2.set_phone1({ W("55"), W("11"), W("1234467890"), ContactQualifier::Business });
        p2.set_email1(Email(W("second@example.com")));

        // Write N persons alternating p1 and p2
        BinaryBuffer bb;
        for (size_t i = 0; i < N; ++i)
        {
            (i % 2 == 0) ? bb.write(p1) : bb.write(p2);
        }

        // Read them back and verify
        Person p3;
        for (size_t i = 0; i < N; ++i)
        {
            bb.read(p3);
            INFO("iteration " << i);
            if (i % 2 == 0)
                CHECK(p3 == p1);
            else
                CHECK(p3 == p2);
        }
    }

    // ===== Text streaming =====

    TEST_CASE("Person text streaming", "[person]")
    {
        PersonName name = { W("John"), W(""), W("Doe") };
        Date dob(1995, 3, 20);
        Person::DataType data = { name, dob };
        Person p(data);

        pd::OutStringStream oss;
        oss << p;
        S text = oss.str();
        // Should contain the name somewhere in the output
        CHECK(text.find(W("John")) != S::npos);
    }
} // namespace pensar_digital::cpplib::contact