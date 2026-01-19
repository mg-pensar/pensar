#ifndef ENDIAN_HPP
#define ENDIAN_HPP

#include "concept.hpp"
#include <cstdint>
#include <bit>

namespace pensar_digital
{
	namespace cpplib
	{
		struct Endian;

		// Endian has 4 int_fast8_t values: UNKNOWN = -1, LITTLE = std::endian::little, BIG = std::endian::big, NATIVE = std::endian::native.
		struct Endian
		{
			int8_t _value;

			static const int8_t NOT_APPLICABLE = -2;
			static const int8_t UNKNOWN        = -1;
			static const int8_t LITTLE         = static_cast<int8_t> (std::endian::little);
			static const int8_t BIG            = static_cast<int8_t> (std::endian::big   );
			static const int8_t NATIVE         = static_cast<int8_t> (std::endian::native);

			// Default constructor
			Endian() = default;

			// Conversion from int_fast8_t
			constexpr Endian (int_fast8_t value) : _value(value) {}

			// Conversion from std::endian
			constexpr Endian (std::endian value) : _value(value == std::endian::little ? LITTLE : (value == std::endian::big ? BIG : NATIVE)) {}

			// Conversion to std::endian
			constexpr operator std::endian() const { return static_cast<std::endian>(_value); }

			// Conversion to int_fast8_t
			constexpr operator int_fast8_t() const { return _value; }
		};

		static_assert(std::is_trivially_copyable<Endian>::value, "Endian must be trivially copyable.");
	} // namespace cpplib
} // namespace pensar_digital

#endif // ENDIAN_HPP
