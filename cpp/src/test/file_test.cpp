// author : Mauricio Gomes
// license: MIT (https://opensource.org/licenses/MIT)

#include <catch2/catch_test_macros.hpp>

#include "../file.hpp"

namespace pensar_digital::cpplib
{
    TEST_CASE("RandomFileNameGenerator", "[file]")
    {
        RandomFileNameGenerator r;
        Path p = r();
        INFO(W("0")); CHECK(p.parent_path() == TMP_PATH.copy_without_trailing_separator());
        S filename = p.filename_only().str();
        INFO(W("1")); CHECK(p.extension() == W(".txt"));
        INFO(W("2")); CHECK(filename.length() == 8);
        INFO(W("3")); CHECK(filename.find_first_not_of(W("0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ")) == S::npos);
        // Check if it does not start with a number.
        INFO(W("4")); CHECK(filename.find_first_of(W("0123456789")) != 0);
    }

    TEST_CASE("TextFile", "[file]")
    {
        Path p;
        {
            p = TMP_PATH / W("text-file-test.txt");
            TmpTextFile file(p, W("blah"));
            p = file.fullpath();

            INFO(W("0")); CHECK(file.exists());
            S s = file.read();
            INFO(W("1")); CHECK(s == W("blah"));
            INFO(W("2")); CHECK(file.remove());
        }
        INFO(W("0")); CHECK(!p.exists());
    }
}
