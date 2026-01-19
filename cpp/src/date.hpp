// author : Mauricio Gomes
// license: MIT (https://opensource.org/licenses/MIT)

#ifndef DATE_HPP
#define DATE_HPP

#include <chrono>

#include "concept.hpp"

namespace pensar_digital
{
    namespace cpplib
    {
        class Date : public std::chrono::year_month_day
        {
            public:
			    Date (int year, int month, int day)
					: std::chrono::year_month_day (std::chrono::year (year), std::chrono::month (month), std::chrono::day (day))
				{
				}

        };
 
        inline static const Date NULL_DATE = Date (0, 0, 0);

        // Ensure the class is trivially copyable and standard layout.
        static_assert (TriviallyCopyable<Date>);

    } // namespace contact
} // namespace pensar_digital

#endif // DATE_HPP
