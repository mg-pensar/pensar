// author : Mauricio Gomes
// license: MIT (https://opensource.org/licenses/MIT)

#ifndef BYTES_UTIL_HPP_INCLUDED
#define BYTES_UTIL_HPP_INCLUDED

#include "constant.hpp"
#include "concept.hpp"

#include <span>
#include <string>
#include <vector>
#include <cstddef>	 // std::byte
#include <bit>		 // std::endian

namespace pensar_digital
{
    namespace cpplib
    {

		/// \brief A method to convert a string into a std::vector<std::byte>.
		/// \param s The string to convert.
		/// \param v The std::vector<std::byte> to convert the string into.
		extern void string_to_bytes (const std::string& s, ByteSpan& v);

		/// \brief A method to convert a string into a std::vector<std::byte>.
		/// \param s The wstring to convert.
		/// \param v The std::vector<std::byte> to convert the string into.
		extern void string_to_bytes (const std::wstring& s, ByteSpan& v);

		///\brief A method to convert a std::span<std::byte> into a string.
		///\param bytes The std::span<std::byte> to convert.
		/// \return A string of the std::span<std::byte>.
		extern void bytes_to_string (const ByteSpan& bytes, std::string& s);

		///\brief A method to convert a std::span<std::byte> into a string.
		///\param bytes The std::span<std::byte> to convert.
		/// \return A string of the std::span<std::byte>.
		extern void bytes_to_wstring (ByteSpan& bytes, std::wstring s);

		///\brief Adds a std::span<std::byte> to a std::span<std::byte>.
		///\param bytes The std::span<std::byte> to add the std::span<std::byte> to.
		/// 
		/// The bytes are added to the end of the dest.
		/// The dest is resized if it is not large enough to hold the data.
		/// \param bytes The std::span<std::byte> to add to the std::span<std::byte>.
		extern void add_bytes (const ByteSpan& bytes, ByteSpan& dest);

		///\brief Adds a string to a std::span<std::byte>.
		///\param bytes The std::span<std::byte> to add the string to.	
		/// 
		/// The string is added to the end of the std::span<std::byte> and the size of the std::span<std::byte> is increased by the size of the string.
		/// The std::span<std::byte> is resized if it is not large enough to hold the string.
		/// \param str The string to add to the std::span<std::byte>.
		extern void add_string_to_bytes (const        std::string& s, ByteSpan& bytes);


		template <typename T>
		void add_number_to_bytes(const T n, ByteSpan& bytes)
		{
			const std::byte* p = reinterpret_cast<const std::byte*>(&n);
			constexpr size_t size = sizeof(T);
			ByteSpan number_bytes(p, p + size);
			add_bytes(number_bytes, bytes);
		}

		extern void int16_t_to_bytes    (const             int16_t i, ByteSpan& bytes);
		extern void int32_t_to_bytes    (const             int32_t i, ByteSpan& bytes);
		extern void int64_t_to_bytes    (const             int64_t i, ByteSpan& bytes);
		extern void int_to_bytes		(const                 int i, ByteSpan& bytes);
		extern void uint16_t_to_bytes	(const            uint16_t i, ByteSpan& bytes);
		extern void uint32_t_to_bytes	(const			  uint32_t i, ByteSpan& bytes);
		extern void uint64_t_to_bytes	(const			  uint64_t i, ByteSpan& bytes);
		extern void uint_to_bytes		(const		  unsigned int i, ByteSpan& bytes);
		extern void float_to_bytes		(const               float f, ByteSpan& bytes);
		extern void double_to_bytes		(const              double d, ByteSpan& bytes);
		extern void bool_to_bytes		(const                bool b, ByteSpan& bytes);
		extern void long_to_bytes		(const                long l, ByteSpan& bytes);
		extern void ulong_to_bytes		(const       unsigned long l, ByteSpan& bytes);
		extern void long_long_to_bytes	(const          long long ll, ByteSpan& bytes);
		extern void ulong_long_to_bytes (const unsigned long long ll, ByteSpan& bytes);

		template <TriviallyCopyable T>
		T bytes_to_type(const ByteSpan& bytes)
		{
			T t;
			std::memcpy(&t, bytes.data(), sizeof(T));
			return t;
		}

    }	// namespace cpplib
}		// namespace pensar_digital

#endif	// BYTES_UTIL_HPP_INCLUDED
