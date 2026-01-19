#ifndef STATISTIC_HPP
#define STATISTIC_HPP

#include <iterator>    // For std::input_iterator, std::iterator_traits
#include <type_traits> // For std::is_arithmetic_v
#include <numeric>     // For std::accumulate
#include <cmath>       // For std::sqrt
#include <limits>      // For std::numeric_limits
#include <ranges>      // For std::ranges::input_range, std::ranges::range_value_t, std::ranges::begin, std::ranges::end

namespace pensar_digital
{
	namespace cpplib
	{

        // Iterator-based function to compute standard deviation
        template <typename Iter>
            requires std::input_iterator<Iter>&& std::is_arithmetic_v<typename std::iterator_traits<Iter>::value_type>
        double standard_deviation(Iter begin, Iter end) {
            using value_type = typename std::iterator_traits<Iter>::value_type;
            struct Accumulator {
                double sum_x = 0.0;
                double sum_x2 = 0.0;
                size_t n = 0;
            };
            Accumulator result = std::accumulate(begin, end, Accumulator{},
                [](Accumulator acc, value_type x) {
                    double dx = static_cast<double>(x);
                    acc.sum_x += dx;
                    acc.sum_x2 += dx * dx;
                    acc.n += 1;
                    return acc;
                });
            if (result.n == 0) {
                return std::numeric_limits<double>::quiet_NaN();
            }
            double mean = result.sum_x / result.n;
            double variance = (result.sum_x2 / result.n) - mean * mean;
            return std::sqrt(variance);
        }

        // Overload to accept the entire collection
        template <typename Collection>
            requires std::ranges::input_range<Collection>&&
        std::is_arithmetic_v<std::ranges::range_value_t<Collection>>
            double standard_deviation(const Collection& collection) {
            return standard_deviation(std::ranges::begin(collection), std::ranges::end(collection));
        }
    }
}

#endif // STATISTIC_HPP
