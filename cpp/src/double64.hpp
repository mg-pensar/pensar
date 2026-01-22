// author : Mauricio Gomes
// license: MIT (https://opensource.org/licenses/MIT)

#ifndef DOUBLE64_HPP
#define DOUBLE64_HPP

#include "constant.hpp"
#include "concept.hpp"

#include <bit>
#include <cstdint>
#include <compare>

namespace pensar_digital
{
    namespace cpplib
    {
        /// \brief A bit-exact, trivially copyable double wrapper with unique object representation.
        /// Stores the IEEE-754 payload as uint64_t.
        struct Double64
        {
            using Bits = uint64_t;
            Bits bits{0};

            constexpr Double64() noexcept = default;
            constexpr explicit Double64(Bits raw_bits) noexcept : bits(raw_bits) {}
            constexpr Double64(double value) noexcept : bits(std::bit_cast<Bits>(value)) {}

            constexpr operator double() const noexcept
            {
                return std::bit_cast<double>(bits);
            }

            constexpr Double64& operator=(double value) noexcept
            {
                bits = std::bit_cast<Bits>(value);
                return *this;
            }

            constexpr Double64& operator+=(double rhs) noexcept
            {
                return *this = static_cast<double>(*this) + rhs;
            }

            constexpr Double64& operator-=(double rhs) noexcept
            {
                return *this = static_cast<double>(*this) - rhs;
            }

            constexpr Double64& operator*=(double rhs) noexcept
            {
                return *this = static_cast<double>(*this) * rhs;
            }

            constexpr Double64& operator/=(double rhs) noexcept
            {
                return *this = static_cast<double>(*this) / rhs;
            }

            friend constexpr Double64 operator+(Double64 lhs, double rhs) noexcept
            {
                lhs += rhs;
                return lhs;
            }

            friend constexpr Double64 operator-(Double64 lhs, double rhs) noexcept
            {
                lhs -= rhs;
                return lhs;
            }

            friend constexpr Double64 operator*(Double64 lhs, double rhs) noexcept
            {
                lhs *= rhs;
                return lhs;
            }

            friend constexpr Double64 operator/(Double64 lhs, double rhs) noexcept
            {
                lhs /= rhs;
                return lhs;
            }

            friend constexpr bool operator==(const Double64& a, const Double64& b) noexcept = default;
            friend constexpr std::strong_ordering operator<=>(const Double64& a, const Double64& b) noexcept = default;
        };

        static_assert(StdLayoutTriviallyCopyableNoPadding<Double64>);
    } // namespace cpplib
} // namespace pensar_digital

#endif // DOUBLE64_HPP
