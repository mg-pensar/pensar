// author : Mauricio Gomes
// license: MIT (https://opensource.org/licenses/MIT)

#ifndef WIRE_DOUBLE_HPP
#define WIRE_DOUBLE_HPP

#include "endian.hpp"
#include "concept.hpp"

#include <bit>
#include <cstdint>
#include <cstring>
#include <limits>
#include <type_traits>
#include <functional>

#if __has_include(<fmt/format.h>)
    #include <fmt/format.h>
    #define CPPLIB_HAS_FMT 1
#else
    #define CPPLIB_HAS_FMT 0
#endif

namespace pensar_digital::cpplib
{
  
    // ------------------------------------------------------------
    // Wire floating-point representation
    // ------------------------------------------------------------

    template<IEEE754Binary T, Endian E = Endian::big()>
    struct Double
    {
        using value_type  = T;
        using bits_type   =
            std::conditional_t<sizeof(T) == 4, std::uint32_t, std::uint64_t>;
        using endian_type = Endian;

        bits_type bits{0};

        // --------------------------------------------------------
        // Construction
        // --------------------------------------------------------

        constexpr Double() noexcept = default;

        constexpr explicit Double(T value) noexcept
            : bits(encode(std::bit_cast<bits_type>(value))) {}

        constexpr explicit Double(bits_type raw_bits) noexcept
            : bits(encode(raw_bits)) {}

        // --------------------------------------------------------
        // Conversion
        // --------------------------------------------------------

        constexpr T value() const noexcept
        {
            return std::bit_cast<T>(decode(bits));
        }

        constexpr operator T() const noexcept
        {
            return value();
        }

        // --------------------------------------------------------
        // Assignment
        // --------------------------------------------------------

        constexpr Double& operator=(T v) noexcept
        {
            bits = encode(std::bit_cast<bits_type>(v));
            return *this;
        }

        // --------------------------------------------------------
        // Arithmetic (value-based)
        // --------------------------------------------------------

        constexpr Double& operator+=(Double rhs) noexcept
        {
            return *this = Double(value() + rhs.value());
        }

        constexpr Double& operator-=(Double rhs) noexcept
        {
            return *this = Double(value() - rhs.value());
        }

        constexpr Double& operator*=(Double rhs) noexcept
        {
            return *this = Double(value() * rhs.value());
        }

        constexpr Double& operator/=(Double rhs) noexcept
        {
            return *this = Double(value() / rhs.value());
        }

        friend constexpr Double operator+(Double a, Double b) noexcept { return a += b; }
        friend constexpr Double operator-(Double a, Double b) noexcept { return a -= b; }
        friend constexpr Double operator*(Double a, Double b) noexcept { return a *= b; }
        friend constexpr Double operator/(Double a, Double b) noexcept { return a /= b; }

        // --------------------------------------------------------
        // Comparisons (numeric, not bitwise)
        // --------------------------------------------------------

        friend constexpr bool operator==(Double a, Double b) noexcept
        {
            return a.value() == b.value();
        }

        friend constexpr auto operator<=>(Double a, Double b) noexcept
        {
            return a.value() <=> b.value();
        }

    private:
        static constexpr bool need_swap =
            (E.value == Endian::LITTLE && std::endian::native == std::endian::big) ||
            (E.value == Endian::BIG    && std::endian::native == std::endian::little);

        static constexpr bits_type encode(bits_type v) noexcept
        {
            if constexpr (need_swap)
                return std::byteswap(v);
            else
                return v;
        }

        static constexpr bits_type decode(bits_type v) noexcept
        {
            if constexpr (need_swap)
                return std::byteswap(v);
            else
                return v;
        }
    };

    // ------------------------------------------------------------
    // Static guarantees
    // ------------------------------------------------------------

    static_assert(WireSafe<Double<double>>);
    static_assert(sizeof(Double<double>) == sizeof(std::uint64_t));
    static_assert(sizeof(Double<float>)  == sizeof(std::uint32_t));

    // ------------------------------------------------------------
    // Useful aliases
    // ------------------------------------------------------------

    using WireDouble = Double<double>;
    using WireFloat  = Double<float>;

    using BEDouble = Double<double, Endian::big()>;
    using LEDouble = Double<double, Endian::little()>;

    using BEFloat  = Double<float,  Endian::big()>;
    using LEFloat  = Double<float,  Endian::little()>;

} // namespace pensar_digital::cpplib

// ------------------------------------------------------------
// std::numeric_limits
// ------------------------------------------------------------

namespace std
{
    template<pensar_digital::cpplib::IEEE754Binary T,
             pensar_digital::cpplib::Endian E>
    struct numeric_limits<pensar_digital::cpplib::Double<T, E>>
        : numeric_limits<T> {};
}

// ------------------------------------------------------------
// std::hash
// ------------------------------------------------------------

namespace std
{
    template<pensar_digital::cpplib::IEEE754Binary T,
             pensar_digital::cpplib::Endian E>
    struct hash<pensar_digital::cpplib::Double<T, E>>
    {
        size_t operator()(const pensar_digital::cpplib::Double<T, E>& d) const noexcept
        {
            return std::hash<typename pensar_digital::cpplib::Double<T, E>::bits_type>{}(d.bits);
        }
    };
}

// ------------------------------------------------------------
// fmt formatter (optional)
// ------------------------------------------------------------

#if CPPLIB_HAS_FMT
template<pensar_digital::cpplib::IEEE754Binary T,
         pensar_digital::cpplib::Endian E>
struct fmt::formatter<pensar_digital::cpplib::Double<T, E>>
    : fmt::formatter<T>
{
    template<typename FormatContext>
    auto format(const pensar_digital::cpplib::Double<T, E>& d,
                FormatContext& ctx) const
    {
        return fmt::formatter<T>::format(static_cast<T>(d), ctx);
    }
};
#endif

#endif // WIRE_DOUBLE_HPP