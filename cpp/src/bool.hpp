#ifndef BOOL_HPP
#define BOOL_HPP

#include "concept.hpp"
#include <cstdint>
#include <ostream>

namespace pensar_digital
{
	namespace cpplib
	{
				// Bool has 3 int_fast8_t states: BOOL_NULL = -1, FALSE = 0, TRUE = 1 .
				struct Bool
				{

					static const int_fast8_t UNKNOWN = -1;
					static const int_fast8_t F = 0;
					static const int_fast8_t T = 1;

					int_fast8_t mvalue = UNKNOWN;
					
					// Default constructor
					Bool() = default;

					// Default copy constructor
					Bool(const Bool&) = default;
					
					// Default move constructor
					Bool(Bool&&) = default;
					
					// Default copy assignment operator
					Bool& operator=(const Bool&) = default;
					
					// Default move assignment operator
					Bool& operator=(Bool&&) = default;
					
					// Default destructor
					~Bool() = default;


					// Conversion from int_fast8_t
					constexpr Bool(int_fast8_t value) : mvalue(value) {}

					// Conversion from bool
					constexpr Bool(bool value) : mvalue(value ? T : F) {}

					// Conversion to bool
					constexpr operator bool() const { return mvalue == T; }

					// Conversion to int_fast8_t
					constexpr operator int_fast8_t() const { return mvalue; }

					// Compare operator
					constexpr bool operator==(const Bool& other) const { return mvalue == other.mvalue; }
				};

				inline std::ostream& operator<<(std::ostream& os, const Bool& b)
				{
					return os << static_cast<int>(b.mvalue);
				}

				inline std::wostream& operator<<(std::wostream& os, const Bool& b)
				{
					return os << static_cast<int>(b.mvalue);
				}

				static_assert(std::is_trivially_copyable<Bool>::value, "Bool must be trivially copyable.");
			} // namespace cpplib
		} // namespace pensar_digital

#endif // BOOL_HPP