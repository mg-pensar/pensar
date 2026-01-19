// author : Mauricio Gomes
// license: MIT (https://opensource.org/licenses/MIT)

#include "../../../unit_test/src/test.hpp"
#include "../file.hpp"

namespace pensar_digital
{
    namespace test = pensar_digital::unit_test;
    using namespace pensar_digital::unit_test;
    namespace cpplib
    {
        TEST(RandomFileNameGenerator, true)
            RandomFileNameGenerator r;
            Path p = r ();
            CHECK_EQ(Path, p.parent_path(), TMP_PATH.copy_without_trailing_separator(), W("0"));
            S filename = p.filename_only().str();
            CHECK_EQ(S, p.extension(), W(".txt"), W("1"));
            CHECK_EQ(size_t, filename.length (), 8, W("2"));
            CHECK(filename.find_first_not_of (W("0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ")) == S::npos, W("3"));
            
            // Check if it does not start with a number.
            CHECK(filename.find_first_of (W("0123456789")) != 0, W("4"));


	
        TEST_END(RandomFileNameGenerator)

        TEST(TextFile, true)
            Path p;
            {
                p = TMP_PATH / W("text-file-test.txt");
                TmpTextFile file(p, W("blah"));
                p = file.fullpath ();
            
                // Checks if file exists.
                CHECK(file.exists(), W("0"));
                S s = file.read();
                CHECK_EQ(S, s, W("blah"), W("1"));

				// Deletes the file.
                CHECK(file.remove(), W("2"));
            }   
            CHECK(! p.exists(), W("0"));
        TEST_END(TextFile)
    }
}