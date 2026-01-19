// author : Mauricio Gomes
// license: MIT (https://opensource.org/licenses/MIT)

#ifndef STOP_WATCH_HPP_INCLUDED
#define STOP_WATCH_HPP_INCLUDED

#include "object.hpp"
#include "s.hpp"
#include <chrono>

namespace pensar_digital
{
    namespace cpplib
    {

    // StopWatch: measures time intervals in nanoseconds by default (but it is configurable) using std::chrono.
	template <typename Resolution = std::chrono::nanoseconds>
    class StopWatch : public virtual Object
    {
		private:
			typedef std::chrono::time_point<std::chrono::steady_clock> TimePoint;
			inline static const TimePoint ZERO = TimePoint ();
			TimePoint mstart;
			TimePoint mlast;
			TimePoint mmark;
			bool mrunning;
		public:
			typedef long long ELAPSED_TYPE;
			static const ELAPSED_TYPE H = 3600000000000;
			static const ELAPSED_TYPE M = 60000000000;
			static const ELAPSED_TYPE S = 1000000000;
			static const ELAPSED_TYPE MS = 1000000;
			static const ELAPSED_TYPE MICRO_SECOND = 1000;

			// Constructor
			StopWatch(bool start_now = true) 
				: mstart (start_now ? std::chrono::steady_clock::now() : ZERO), mlast (ZERO), mrunning (start_now) {}

			// Restart the timer
			inline void start () { mstart = std::chrono::steady_clock::now(); mrunning = true; }

			// Reset the timer.
			inline void reset () { mstart = std::chrono::steady_clock::now(); mlast = ZERO; mrunning = true; }

			inline void stop () { mlast = std::chrono::steady_clock::now(); mrunning = false; }

			// Get elapsed time in nanoseconds
			inline ELAPSED_TYPE elapsed ()
			{
				return std::chrono::duration_cast<Resolution> (mrunning ? std::chrono::steady_clock::now() - mstart : mlast - mstart).count ();
			}

			inline void mark () { mmark = std::chrono::steady_clock::now(); }	

			inline ELAPSED_TYPE elapsed_since_mark ()
			{
				return std::chrono::duration_cast<Resolution> (mrunning ? std::chrono::steady_clock::now() - mmark : mlast - mmark).count();
			}

			inline ELAPSED_TYPE now ()
			{
				return std::chrono::duration_cast<Resolution> (std::chrono::steady_clock::now() - ZERO).count();
			}

			pensar_digital::cpplib::S elapsed_formatted(ELAPSED_TYPE elapsed_nanoseconds)
			{
				ELAPSED_TYPE elapsed = elapsed_nanoseconds;
				ELAPSED_TYPE hours = elapsed / H;
				elapsed -= hours * H;
				ELAPSED_TYPE minutes = elapsed / M;
				elapsed -= minutes * M;
				ELAPSED_TYPE seconds = elapsed / S;
				elapsed -= seconds * S;
				ELAPSED_TYPE milliseconds = elapsed / MS;
				if (milliseconds > 1000)
					std::cerr << "Warning: milliseconds > 1000" << std::endl;
				elapsed -= milliseconds * MS;
				ELAPSED_TYPE microseconds = elapsed / MICRO_SECOND;
				elapsed -= microseconds * MICRO_SECOND;

				pensar_digital::cpplib::S result = pd::pad_left0 (hours, 2) + W(":");
				result += pd::pad_left0 (minutes, 2) + W(":");
				result += pd::pad_left0 (seconds, 2) + W(".");
				result += pd::pad_left0 (milliseconds, 3) + W(".");
				result += pd::pad_left0 (microseconds, 3) + W(".");
				result += pd::pad_left0 (elapsed, 3);

				return result;
			}

			// Get elapsed as a formatted string (hh:mm:ss.mmm)
			pensar_digital::cpplib::S elapsed_formatted ()
			{
				return elapsed_formatted (elapsed ());
			}
			pensar_digital::cpplib::S elapsed_since_mark_formatted ()
			{
				return elapsed_formatted(elapsed_since_mark ());
			}
	};


	} // namespace cpplib
} // namespace pensar_digitalw
#endif // STOP_WATCH_HPP_INCLUDED
