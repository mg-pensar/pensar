// include guard to avoid multiple inclusions.
#ifndef SYSINFO_HPP
#define SYSINFO_HPP

#include <vector>
#include <thread>
#include <chrono>
#include <atomic>
#include <cmath>
#include <iostream>
#include "code_util.hpp"

namespace pensar_digital 
{
	namespace cpplib 
	{
		using EstimateCPUCoresResult = Result<unsigned int>;
        // Function to estimate the number of CPU cores
        auto estimate_cpu_threads ()
        {
            // Maximum number of threads to test (to avoid excessive resource usage)
            const unsigned int max_threads = std::thread::hardware_concurrency() * 2;
            // Minimum number of threads to start with
            const unsigned int min_threads = 1;
            // Workload duration per thread (adjust for sufficient measurement time)
            const double workload_duration_ms = 100.0;
            // Threshold for performance degradation (percentage increase in time)
            const double degradation_threshold = 1.20; // 20% 

            std::vector<double> execution_times(max_threads + 1, 0.0);

            // Function for each thread to execute (compute-intensive task)
            auto worker = [](std::atomic<bool>& running, double duration_ms) {
                auto start = std::chrono::high_resolution_clock::now();
                double dummy = 0.0;
                // Tight loop with some computation to keep CPU busy
                while (running) {
                    for (int i = 0; i < 100; ++i) {
                        dummy += std::sin(std::cos(dummy + i));
                    }
                    // Check if we exceeded duration
                    auto now = std::chrono::high_resolution_clock::now();
                    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - start).count();
                    if (elapsed >= duration_ms) {
                        break;
                    }
                }
                // Prevent optimization
                if (dummy == 0.0) std::cout << "";
                };

            // Test with increasing number of threads
            for (unsigned int num_threads = min_threads; num_threads <= max_threads; ++num_threads) {
                std::vector<std::thread> threads;
                std::atomic<bool> running(true);

                // Start timing
                auto start = std::chrono::high_resolution_clock::now();

                // Spawn threads
                for (unsigned int i = 0; i < num_threads; ++i) {
                    threads.emplace_back(worker, std::ref(running), workload_duration_ms);
                }

                // Wait for all threads to complete
                for (auto& t : threads) {
                    t.join();
                }
                running = false;

                // Measure total execution time
                auto end = std::chrono::high_resolution_clock::now();
                execution_times[num_threads] = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count() / 1000.0;

                // Avoid excessive testing
                if (num_threads > 1) {
                    double prev_time = execution_times[num_threads - 1];
                    double curr_time = execution_times[num_threads];

                    // Check for performance degradation
                    if (curr_time > prev_time * degradation_threshold) 
                    {
                        return num_threads; // Previous thread count is likely the core count so we would
                        // need to subtract 1. However one thread is already in use to run the test so
                        // not subtracting one is a better estimate.
                    }
                }
            }
            return max_threads; // If no degradation detected, return max tested threads
        }

		// Returns stdstd::thread::hardware_concurrency() if result > 0, if not estimates
		// it by running estimate_cpu_threads function n times and returning the most common result.
        auto cpu_threads(const unsigned int n = 10)
        {
            auto result = std::thread::hardware_concurrency();
            
            if (result > 0)
            {
                return result;
            }
            
            // Run estimate_cpu_threads function n times and returns the most common result.
            std::vector<unsigned int> results(n);
            for (unsigned int i = 0; i < n; ++i)
            {
                results[i] = estimate_cpu_threads();

                std::this_thread::sleep_for(std::chrono::milliseconds(50)); // Sleep to avoid excessive CPU usage
            }
            // returns the statistic mode of the array.
            std::sort(results.begin(), results.end());
            unsigned int mode = results[0];
            unsigned int count = 1;
            unsigned int max_count = 1;
            for (unsigned int i = 1; i < n; ++i)
            {
                if (results[i] == results[i - 1])
                {
                    count++;
                }
                else
                {
                    if (count > max_count)
                    {
                        max_count = count;
                        mode = results[i - 1];
                    }
                    count = 1;
                }
            }
            if (count > max_count)
            {
                mode = results[n - 1];
            }
            return mode;
        }
                    

    }  // namespace cpplib
}  // namespace pensar_digital

#endif // SYSINFO_HPP
