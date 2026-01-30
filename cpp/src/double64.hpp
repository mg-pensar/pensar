// author : Mauricio Gomes
// license: MIT (https://opensource.org/licenses/MIT)

#ifndef DOUBLE64_HPP
#define DOUBLE64_HPP

#include <bit>
#include <cstdint>
#include <compare>
#include <limits>
#include <type_traits>
#include <functional>

// ------------------------------------------------------------
// Optional fmt support (auto-detected)
// ------------------------------------------------------------

#if __has_include(<fmt/format.h>)
    #include <fmt/format.h>
    #define CPPLIB_HAS_FMT 1
#else
    #define CPPLIB_HAS_FMT 0
#endif

#include "constant.hpp"
#include "concept.hpp"

namespace pensar_digital::cpplib
{
    // ------------------------------------------------------------
    // endian helpers
    // ------------------------------------------------------------

    [[nodiscard]] constexpr uint64_t bswap64(uint64_t v) noexcept
    {
        return  ((v & 0x00000000000000FFull) << 56) |
                ((v & 0x000000000000FF00ull) << 40) |
                ((v & 0x0000000000FF0000ull) << 24) |
                ((v & 0x00000000FF000000ull) <<  8) |
                ((v & 0x000000FF00000000ull) >>  8) |
                ((v & 0x0000FF0000000000ull) >> 24) |
                ((v & 0x00FF000000000000ull) >> 40) |
                ((v & 0xFF00000000000000ull) >> 56);
    }

    [[nodiscard]] constexpr uint64_t native_to_be(uint64_t v) noexcept
    {
        if constexpr (std::endian::native == std::endian::little)
            return bswap64(v);
        else
            return v;
    }

    [[nodiscard]] constexpr uint64_t be_to_native(uint64_t v) noexcept
    {
        if constexpr (std::endian::native == std::endian::little)
            return bswap64(v);
        else
            return v;
    }

    // ------------------------------------------------------------
    // Double64
    // ------------------------------------------------------------

    struct Double64
    {
        using Bits = uint64_t;
        Bits bits{0}; // always stored big-endian

        // ---- construction ----

        constexpr Double64() noexcept = default;

        constexpr explicit Double64(double value) noexcept
            : bits(native_to_be(std::bit_cast<Bits>(value)))
        {}

        // construct directly from big-endian bits (advanced use)
        constexpr explicit Double64(Bits raw_be_bits, std::in_place_t) noexcept
            : bits(raw_be_bits)
        {}

        // ---- observation ----

        [[nodiscard]] constexpr double value() const noexcept
        {
            return std::bit_cast<double>(be_to_native(bits));
        }

        // implicit conversion (observation only)
        constexpr operator double() const noexcept
        {
            return value();
        }

        // ---- arithmetic (Double64 ⨯ Double64) ----

        constexpr Double64& operator+=(const Double64& rhs) noexcept
        {
            return *this = Double64(value() + rhs.value());
        }

        constexpr Double64& operator-=(const Double64& rhs) noexcept
        {
            return *this = Double64(value() - rhs.value());
        }

        constexpr Double64& operator*=(const Double64& rhs) noexcept
        {
            return *this = Double64(value() * rhs.value());
        }

        constexpr Double64& operator/=(const Double64& rhs) noexcept
        {
            return *this = Double64(value() / rhs.value());
        }

        // ---- compound assignment (Double64 ⨯ double) ----

        constexpr Double64& operator+=(double rhs) noexcept
        {
            return *this = Double64(value() + rhs);
        }

        constexpr Double64& operator-=(double rhs) noexcept
        {
            return *this = Double64(value() - rhs);
        }

        constexpr Double64& operator*=(double rhs) noexcept
        {
            return *this = Double64(value() * rhs);
        }

        constexpr Double64& operator/=(double rhs) noexcept
        {
            return *this = Double64(value() / rhs);
        }

        friend constexpr Double64 operator+(Double64 a, const Double64& b) noexcept
        {
            a += b;
            return a;
        }

        friend constexpr Double64 operator-(Double64 a, const Double64& b) noexcept
        {
            a -= b;
            return a;
        }

        friend constexpr Double64 operator*(Double64 a, const Double64& b) noexcept
        {
            a *= b;
            return a;
        }

        friend constexpr Double64 operator/(Double64 a, const Double64& b) noexcept
        {
            a /= b;
            return a;
        }

        // ---- comparisons ----

        friend constexpr bool operator==(const Double64&, const Double64&) noexcept = default;
        friend constexpr std::strong_ordering operator<=>(const Double64&, const Double64&) noexcept = default;
    };

    // ------------------------------------------------------------
    // Safety guarantees
    // ------------------------------------------------------------

    static_assert(std::is_trivially_copyable_v<Double64>);
    static_assert(std::has_unique_object_representations_v<Double64>);
    static_assert(sizeof(Double64) == sizeof(uint64_t));

} // namespace pensar_digital::cpplib

// ------------------------------------------------------------
// std::numeric_limits specialization
// ------------------------------------------------------------

template<>
struct std::numeric_limits<pensar_digital::cpplib::Double64>
{
    static constexpr bool is_specialized = true;

    static constexpr pensar_digital::cpplib::Double64 min() noexcept
    {
        return pensar_digital::cpplib::Double64(
            std::numeric_limits<double>::min());
    }

    static constexpr pensar_digital::cpplib::Double64 max() noexcept
    {
        return pensar_digital::cpplib::Double64(
            std::numeric_limits<double>::max());
    }

    static constexpr pensar_digital::cpplib::Double64 lowest() noexcept
    {
        return pensar_digital::cpplib::Double64(
            std::numeric_limits<double>::lowest());
    }

    static constexpr int digits   = std::numeric_limits<double>::digits;
    static constexpr int digits10 = std::numeric_limits<double>::digits10;
    static constexpr bool is_signed  = true;
    static constexpr bool is_integer = false;
    static constexpr bool is_exact   = false;
    static constexpr int radix       = 2;
    static constexpr bool is_iec559  = true;
};

// ------------------------------------------------------------
// std::hash specialization (canonical, endian-stable)
// ------------------------------------------------------------

template<>
struct std::hash<pensar_digital::cpplib::Double64>
{
    size_t operator()(const pensar_digital::cpplib::Double64& d) const noexcept
    {
        return std::hash<uint64_t>{}(d.bits);
    }
};

// ------------------------------------------------------------
// fmt::formatter (only if fmt is available)
// ------------------------------------------------------------

#if CPPLIB_HAS_FMT
template<>
struct fmt::formatter<pensar_digital::cpplib::Double64>
    : fmt::formatter<double>
{
    template<typename FormatContext>
    auto format(const pensar_digital::cpplib::Double64& d,
                FormatContext& ctx) const
    {
        return fmt::formatter<double>::format(
            static_cast<double>(d), ctx);
    }
};
#endif

#endif // DOUBLE64_HPP