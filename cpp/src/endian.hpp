#ifndef ENDIAN_HPP
#define ENDIAN_HPP

#include <bit>
#include <cstdint>
#include <type_traits>

namespace pensar_digital::cpplib
{
    struct Endian
    {
        using value_type = int8_t;

        value_type value{UNKNOWN};

        // Stable, library-defined values
        static constexpr value_type NOT_APPLICABLE = -2;
        static constexpr value_type UNKNOWN        = -1;
        static constexpr value_type LITTLE         =  0;
        static constexpr value_type BIG            =  1;
        static constexpr value_type NATIVE         =
            (std::endian::native == std::endian::little) ? LITTLE :
            (std::endian::native == std::endian::big   ) ? BIG    :
                                                           UNKNOWN;

        constexpr Endian() = default;
        constexpr explicit Endian(value_type v) : value(v) {}

        // Named constructors (recommended)
        static constexpr Endian little() noexcept { return Endian(LITTLE); }
        static constexpr Endian big()    noexcept { return Endian(BIG); }
        static constexpr Endian native() noexcept { return Endian(NATIVE); }
        static constexpr Endian unknown() noexcept { return Endian(UNKNOWN); }

        // Queries
        constexpr bool is_little() const noexcept { return value == LITTLE; }
        constexpr bool is_big()    const noexcept { return value == BIG; }
        constexpr bool is_native() const noexcept { return value == NATIVE; }
        constexpr bool known()     const noexcept { return value >= 0; }

        // Explicit conversion (safe)
        constexpr std::endian to_std() const noexcept
        {
            return value == LITTLE ? std::endian::little :
                   value == BIG    ? std::endian::big    :
                                      std::endian::native; // fallback
        }

        // Comparisons
        friend constexpr bool operator==(Endian a, Endian b) noexcept
        {
            return a.value == b.value;
        }
    };

    static_assert(std::is_trivially_copyable_v<Endian>);
    static_assert(sizeof(Endian) == 1);

} // namespace pensar_digital::cpplib

#endif // ENDIAN_HPP
