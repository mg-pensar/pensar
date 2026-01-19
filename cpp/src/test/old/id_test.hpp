#ifndef ID_TEST_HPP_INCLUDED
#define ID_TEST_HPP_INCLUDED

#include "../id.hpp"
#include "../../unit_test/test.hpp"

#include <filesystem>
#include <iostream>
#include <fstream>

namespace pensar_digital
{
    namespace cpplib
    {
        using namespace pensar_digital::unit_test;
        namespace fs = std::filesystem;

       class Dummy: public Id<int>
        {
          public:
            Dummy (const int aid = -1): Id<int>(aid) {};
        };

        TEST(GetSet)
            Dummy id;
            int expected = -1;
            CHECK_EQ(int64_t, id.get_id (), expected, "0");
            expected = 10;
            id.set_id(expected);
            CHECK_EQ (int64_t, id.get_id (), expected, "1");

            expected = 5;
            Dummy id2(expected);
            CHECK_EQ (int64_t, id2.get_id (), expected, "2");
        TEST_END

        TEST(EqOperator)
            Dummy id;
            Dummy id2;
            CHECK (id == id2, "3");

            id.set_id (1);
            CHECK (! (id == id2), "4");
        TEST_END

        TEST(DiffOperator)
            Dummy id;
            Dummy id2;
            CHECK (!(id != id2), "5");

            id.set_id (1);
            CHECK (id != id2, "6");
        TEST_END

        TEST(IdSerialization)
            fs::path dummy_file = fs::temp_directory_path() / "id.txt";
            Dummy d1(7);
            std::fstream f;
            dummy_file << id;
            Dummy d2;
            dummy_file >>
        TEST_END
    }
}

#endif // ID_TEST_HPP_INCLUDED
