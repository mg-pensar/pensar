// author : Mauricio Gomes
// license: MIT (https://opensource.org/licenses/MIT)

#ifndef WIRE_INT_HPP
#define WIRE_INT_HPP

#include "endian.hpp"
#include "concept.hpp"

#include <bit>
#include <cstdint>
#include <type_traits>

namespace pensar_digital::cpplib
{
    
    
    // ------------------------------------------------------------
    // Endian-aware integer
    // ------------------------------------------------------------

    template<IntegerLike T, Endian E>
    struct Int
    {
        using value_type = T;
        using endian_type = Endian;

        value_type storage{0};

        // --------------------------------------------------------
        // Construction
        // --------------------------------------------------------

        constexpr Int() = default;

        // Explicit: prevent accidental mixing
        constexpr explicit Int(T value) noexcept
            : storage(encode(value)) {}

        // --------------------------------------------------------
        // Conversion
        // --------------------------------------------------------

        constexpr T value() const noexcept
        {
            return decode(storage);
        }

        constexpr operator T() const noexcept
        {
            return value();
        }

        // --------------------------------------------------------
        // Assignment
        // --------------------------------------------------------

        constexpr Int& operator=(T v) noexcept
        {
            storage = encode(v);
            return *this;
        }

        // --------------------------------------------------------
        // Comparisons (value-based)
        // --------------------------------------------------------

        friend constexpr bool operator==(const Int& a, const Int& b) noexcept
        {
            return a.value() == b.value();
        }

        friend constexpr auto operator<=>(const Int& a, const Int& b) noexcept
        {
            return a.value() <=> b.value();
        }

    private:
        static constexpr bool need_swap =
            (E.value == Endian::LITTLE && std::endian::native == std::endian::big) ||
            (E.value == Endian::BIG    && std::endian::native == std::endian::little);

        static constexpr value_type encode(value_type v) noexcept
        {
            if constexpr (need_swap)
                return std::byteswap(v);
            else
                return v;
        }

        static constexpr value_type decode(value_type v) noexcept
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

    template<IntegerLike T, Endian E>
    inline constexpr bool is_wire_safe_v =
        WireSafe<Int<T, E>>;

    static_assert(sizeof(Int<std::uint32_t, Endian::big()>) == sizeof(std::uint32_t));

    using WireInt32 = Int<std::uint32_t, Endian::native()>;
    using WireInt64 = Int<std::uint64_t, Endian::native()>;
    
} // namespace pensar_digital::cpplib

#endif // WIRE_INT_HPP