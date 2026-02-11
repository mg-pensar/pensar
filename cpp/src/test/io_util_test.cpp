#include <catch2/catch_test_macros.hpp>

#include <memory>

#include "../s.hpp"
#include "../object.hpp"
#include "../io_util.hpp"
#include "../sys_user_info.hpp"
#include "../file.hpp"

namespace pensar_digital::cpplib
{
    TEST_CASE("PathTest", "[io_util]")
    {
        Path temp_dir = get_user_home() / W("test_dir");
        Path file1 = temp_dir / W("file_name");
        Path file2 = temp_dir / W("dir" / "file_name");
        TextFile tf(file1, W("blah"));
        INFO(W("0")); CHECK(tf.exists());
        TextFile tf2(file2, W("blah"));
        INFO(W("0")); CHECK(tf2.exists());
        fs::last_write_time(file2, last_write_time(file1));
    }
}
