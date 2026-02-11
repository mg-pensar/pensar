// $Id$

#include <catch2/catch_test_macros.hpp>

#include "../io_util.hpp"
#include "../log.hpp"
#include "../file.hpp"

namespace pensar_digital::cpplib
{
#ifdef LOG_ON

    TEST_CASE("Log", "[log]")
    {
        enable_log();
        LOG(W("Logging is cool and efficient."));

        Path p1(default_log_file_name());

        INFO("0"); CHECK(p1.exists());

        disable_log();
        p1.remove();

        LOG(W("nope"));
        INFO("1"); CHECK(!p1.exists());

        enable_log();
        LOG(W("nope"));
        INFO("2"); CHECK(p1.exists());
    }
#endif
}
