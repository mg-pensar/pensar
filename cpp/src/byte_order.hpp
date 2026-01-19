// author : Mauricio Gomes
// license: MIT (https://opensource.org/licenses/MIT)

#ifndef BYTE_ORDER_HPP
#define BYTE_ORDER_HPP

#include <span>
#include <bit>

namespace pensar_digital
{
    namespace cpplib
    {
        namespace pd = pensar_digital::cpplib;
        enum Invariance : bool { DATA_INVARIANCE = false, ADDRESS_INVARIANCE = true};
        typedef int_fast8_t Word;
        constexpr Word WORD_SIZE = sizeof (Word);

        // Perfect swap using move semantics.
        template <typename T>
		inline void swap (T& a, T& b) noexcept
		{
			T tmp = std::move (a);
			a = std::move (b);
			b = std::move (tmp);
		}

        struct ByteOrder
        {
            std::endian mendian       = std::endian::native   ; //!< Member variable "endian"
            Invariance  minvariance   = ADDRESS_INVARIANCE    ; //!< Member variable "invariance"
            Word        maddress_size = sizeof (std::byte); //!< Member variable "size" in bytes.
            
            ByteOrder (const std::endian endian       = std::endian::native, 
                       const Invariance  invariance   =  ADDRESS_INVARIANCE, 
                       const Word        address_size = sizeof (Word)) noexcept :
				mendian (endian), minvariance (invariance), maddress_size (address_size) {}

            inline bool is_address_invariant () const noexcept { return minvariance ==  ADDRESS_INVARIANCE; }
            inline bool is_data_invariant    () const noexcept { return minvariance ==     DATA_INVARIANCE; }
            inline bool is_little_endian     () const noexcept { return mendian == std::endian::little; }
            inline bool is_big_endian        () const noexcept { return mendian == std::endian::big   ; }
            inline bool is_native_endian     () const noexcept { return mendian == std::endian::native; }

            inline bool equals_invariance   (const ByteOrder& other) const noexcept { return minvariance   == other.minvariance; }
            inline bool equals_endian       (const ByteOrder& other) const noexcept { return mendian       == other.mendian    ; }
            inline bool equals_address_size (const ByteOrder& other) const noexcept { return maddress_size == other.maddress_size; }
            inline bool only_endian_differs (const ByteOrder& other) const noexcept { return ((!equals_endian(other)) && (equals_invariance (other) && equals_address_size (other))); }
            
            inline bool equals  (const ByteOrder& other) const noexcept 
            { 
                return equals_invariance (other) && equals_endian(other) && equals_address_size (other);
            }

			inline bool operator == (const ByteOrder& other) const noexcept { return  equals (other); }
            inline bool operator != (const ByteOrder& other) const noexcept { return !equals (other); }
        };
        inline const ByteOrder little_address_8_byte_order = ByteOrder (std::endian::little, ADDRESS_INVARIANCE, sizeof (std::byte) * 8);
        inline const ByteOrder big_address_8_byte_order    = ByteOrder (std::endian::big   , ADDRESS_INVARIANCE, sizeof (std::byte) * 8);
        inline const ByteOrder native_byte_order           = ByteOrder (std::endian::native, ADDRESS_INVARIANCE, sizeof (std::byte) * 8);

        template < size_t sz = 18>
        void convert(std::span<std::byte>& data, size_t original_data_size, const ByteOrder& from, const ByteOrder& to) noexcept
        {
            if ((from != to) && (data.size() > 1) && from.only_endian_differs(to) && (original_data_size > 1))
            {
                auto e = data.size() - original_data_size;
                for (std::span<std::byte>::size_type i = 0; i <= e; i += original_data_size)
                {
                    auto x = original_data_size / 2;
                    for (auto j = 0; j < x; ++j)
                    {
                        pd::swap<std::byte>(data[i + j], data[i - j + original_data_size - 1]);
                    }
                }
            }

        } // convert
        
        inline void convert(const std::span<std::byte>& from_data, std::span<std::byte>& to_data, const ByteOrder& from, const ByteOrder& to) noexcept
        {

        } // convert


    } // namespace cpplib
} // namespace pensar_digital
#endif // BYTE_ORDER_HPP

