#include <memory>\


#include "../../../unit_test/src/test.hpp"
#include "../s.hpp"
#include "../object.hpp"
#include "../io_util.hpp"
#include "../sys_user_info.hpp"
#include "../file.hpp"



namespace pensar_digital
{
    using namespace pensar_digital::unit_test;
    namespace cpplib
    {
        TEST(PathTest, true)
            Path temp_dir = get_user_home() / W("test_dir");
            Path file1 = temp_dir / W("file_name");
            Path file2 = temp_dir / W("dir" / "file_name");
            TextFile tf(file1, W("blah"));
            CHECK(tf.exists(), W("0"));
            TextFile tf2(file2, W("blah"));
            CHECK(tf2.exists(), W("0"));
            fs::last_write_time (file2, last_write_time (file1)); // Do timestamp file2 = timestamp file2.
            //CHECK_EQ(Path, file1, file2, "2");

            /*fs::last_write_time(file2, last_write_time(file1) + (time_t)60);
            BOOST_CHECK (! cpp::is_same (file1, file2));

            Path file3 = temp_dir / "file_name3";
            cpp::create_file (file3, "blah");

            BOOST_CHECK (cpp::is_same (file1, file3, cpp::SAME_SIZE + cpp::SAME_TIME));
            */
            TEST_END(PathTest)
    }
}
