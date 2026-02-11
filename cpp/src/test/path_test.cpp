// author : Mauricio Gomes
// license: MIT (https://opensource.org/licenses/MIT)

#include <catch2/catch_test_macros.hpp>

#include "../path.hpp"
#include "sys_user_info.hpp"

namespace pensar_digital::cpplib
{
    TEST_CASE("Path", "[path]")
    {
        static_assert(TriviallyCopyable<CPath>);
        Path path;
        INFO(W("0")); CHECK(path == CURRENT_DIR);

        path = W("/");
        INFO(W("1")); CHECK(path == path.root_path());

        Path home = get_user_home();
        path = home + W("/path_test/");
        INFO(W("1.1")); CHECK(path.parent_path() == home + W("/path_test"));
    }
}
