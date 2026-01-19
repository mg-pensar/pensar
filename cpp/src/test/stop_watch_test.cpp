// author : Mauricio Gomes
// license: MIT (https://opensource.org/licenses/MIT)

#include "../../../unit_test/src/test.hpp"
#include "../s.hpp"
#include "../stop_watch.hpp"
#include "../constraint.hpp"
#include <thread>
#include <chrono>

namespace pensar_digital
{
    namespace test = pensar_digital::unit_test;
    using namespace pensar_digital::unit_test;
    namespace cpplib
    {
        TEST(StopWatch, true)
            StopWatch<> sp;
            StopWatch<>::ELAPSED_TYPE T = 2 * StopWatch<>::MS;

            std::this_thread::sleep_for(std::chrono::nanoseconds(T));
            sp.mark ();
            sp.stop ();
            StopWatch<>::ELAPSED_TYPE elapsed = sp.elapsed ();   
            CHECK(elapsed >= T, W("0."));
            StopWatch<>::ELAPSED_TYPE mark_elapsed = sp.elapsed_since_mark ();
            S elapsed_formatted = sp.elapsed_formatted ();
            S elapsed_since_mark_formatted = sp.elapsed_since_mark_formatted ();
            CHECK(mark_elapsed < elapsed, W("1. elapsed = ") + elapsed_formatted + W(" elapsed_since_mark = ") + elapsed_since_mark_formatted);
            //out () << "elapsed = " << elapsed_formatted << " elapsed_since_mark = " << elapsed_since_mark_formatted << std::endl;
         TEST_END(StopWatch)

        /*
        TEST(Over1000msBug, true)
            const long long UmSegundo = 1000000000;
			const long long DoisSegundos = 2 * UmSegundo;
			StopWatch<> sw;
		    sw.mark();
		    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
		    sw.stop();
		    StopWatch<>::ELAPSED_TYPE elapsed = sw.elapsed();
		    StopWatch<>::ELAPSED_TYPE mark_elapsed = sw.elapsed_since_mark();
		    S elapsed_formatted = sw.elapsed_formatted();
		    S elapsed_since_mark_formatted = sw.elapsed_since_mark_formatted();
		    CHECK(mark_elapsed < elapsed, W("1. elapsed = ") + elapsed_formatted + W(" elapsed_since_mark = ") + elapsed_since_mark_formatted);
		    CHECK(elapsed >= UmSegundo, W("0. elapsed = ") + elapsed_formatted);
		    CHECK(elapsed < DoisSegundos, W("2. elapsed = ") + elapsed_formatted);
		TEST_END(Over1000msBug)
        */
    }
}
