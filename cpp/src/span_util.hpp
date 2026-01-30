// Guard
#ifndef SPAN_UTIL_HPP
#define SPAN_UTIL_HPP

#include <span>         // for std::span
#include <type_traits>  // for std::is_trivially_copyable_v, std::has_unique_object_representations_v
#include <ranges>       // for std::ranges::contiguous_range, std::ranges::sized_range
#include <cstddef>      // std::byte

namespace pensar_digital
{
    namespace cpplib
    {
        namespace pd = pensar_digital::cpplib;

        template<typename T>
        concept MemcmpSafe =
            std::is_trivially_copyable_v<T> &&
            std::has_unique_object_representations_v<T>;

        template<WireSafe T>
        inline std::span<const std::byte> bytes(T* ptr) noexcept 
        {
                return std::as_bytes(std::span{ (BytePtr)ptr, sizeof(T) });
        }
            
        /// \brief Uses std::as_writable_bytes to get a span of writable bytes from the object.
        template<WireSafe T>
        inline std::span<std::byte> wbytes(T* ptr) noexcept
        {
            static_assert (sizeof(char) == sizeof(std::byte));

            return std::as_writable_bytes(bytes(ptr));
        }

        template<typename C>
        concept ContiguousContainer =
            std::ranges::contiguous_range<C> &&
            std::ranges::sized_range<C>;            

        template<ContiguousContainer C>
        requires MemcmpSafe<std::ranges::range_value_t<C>>
        std::span<std::byte> ccbytes(C& c) noexcept
        {
            using T = std::ranges::range_value_t<C>;

            return 
            {
                reinterpret_cast<std::byte*>(std::ranges::data(c)),
                std::ranges::size(c) * sizeof(T)
            };
        }

        template<ContiguousContainer C>
        requires MemcmpSafe<std::ranges::range_value_t<C>>
        std::span<const std::byte> ccbytes(const C& c) noexcept
        {
            using T = std::ranges::range_value_t<C>;

            return {
                reinterpret_cast<const std::byte*>(std::ranges::data(c)),
                std::ranges::size(c) * sizeof(T)
            };
        }       
    } // namespace cpplib
} // namespace pensar_digital
#endif // SPAN_UTIL_HPP