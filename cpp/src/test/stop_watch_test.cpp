// author : Mauricio Gomes
// license: MIT (https://opensource.org/licenses/MIT)

#include <catch2/catch_test_macros.hpp>

#include "../s.hpp"
#include "../stop_watch.hpp"
#include "../constraint.hpp"
#include <thread>
#include <chrono>

namespace pensar_digital::cpplib
{
    TEST_CASE("StopWatch", "[stopwatch]")
    {
        StopWatch<> sp;
        StopWatch<>::ELAPSED_TYPE T = 2 * StopWatch<>::MS;

        std::this_thread::sleep_for(std::chrono::nanoseconds(T));
        sp.mark();
        sp.stop();
        StopWatch<>::ELAPSED_TYPE elapsed = sp.elapsed();
        INFO(W("0.")); CHECK(elapsed >= T);
        StopWatch<>::ELAPSED_TYPE mark_elapsed = sp.elapsed_since_mark();
        S elapsed_formatted = sp.elapsed_formatted();
        S elapsed_since_mark_formatted = sp.elapsed_since_mark_formatted();
        INFO(W("1. elapsed = ") + elapsed_formatted + W(" elapsed_since_mark = ") + elapsed_since_mark_formatted);
        CHECK(mark_elapsed < elapsed);
    }
}
