// $Id$

#include "../../../unit_test/src/test.hpp"


#include "../io_util.hpp"

#include "../log.hpp"

#include "../file.hpp"

namespace pensar_digital
{
    using namespace pensar_digital::unit_test;
    namespace cpplib
    {
#ifdef LOG_ON

        TEST(Log, true)
        enable_log();
        LOG(W("Logging is cool and efficient."));

        Path p1(default_log_file_name ());

        CHECK(p1.exists (), "0");

        disable_log();
        p1.remove();

        LOG(W("nope"));
        CHECK(!p1.exists(), "1");

        enable_log();
        LOG(W("nope"));
        CHECK(p1.exists(), "2");
#endif
		TEST_END(Log)
    }
}
