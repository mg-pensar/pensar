#ifndef CS_HPP
#define CS_HPP

#include <array>
#include <cstring>
#include <string>
#include <stdexcept>
#include <string_view>
#include <iostream>
#include <sstream>

#include "string_def.hpp"

namespace pensar_digital
{
    namespace cpplib
    {
        // Forward declaration to break circular include dependency
        namespace icu_util
        {
            template<typename CharT>
            void remove_accent_char(CharT* c) noexcept;
        }
        using S = std::basic_string<C>;
        using SView = std::basic_string_view<C>;
        using SIter = std::basic_string<C>::iterator;
        using SConstIter = std::basic_string<C>::const_iterator;
        using InStream = std::basic_istream<C>;
        using OutStream = std::basic_ostream<C>;
        using SStream = std::basic_stringstream<C>;
        using InStreamBuf = std::basic_streambuf<C>;
        using OutStreamBuf = std::basic_streambuf<C>;
        using InStreamBufIter = std::istreambuf_iterator<C>;
        using OutStreamBufIter = std::ostreambuf_iterator<C>;
        using InStringStream = std::basic_istringstream<C>;
        using OutStringStream = std::basic_ostringstream<C>;
        using InFStream = std::basic_ifstream<C>;
        using OutFStream = std::basic_ofstream<C>;
        using FStream = std::basic_fstream<C>;
        using InFileBuf = std::basic_filebuf<C>;
        using InFileBufIter = std::istreambuf_iterator<C>;
        using OutFileBuf = std::basic_filebuf<C>;
        using OutFileBufIter = std::ostreambuf_iterator<C>;


        /// Remove accent from a single character using ICU normalization.
        /// Handles all Unicode accented characters automatically.
        inline void remove_accent(C* c) noexcept
        {
            icu_util::remove_accent_char(c);
        }


        inline C copy_remove_accent(const C c) noexcept
        {
            C ch = c;
            remove_accent(&ch);
            return ch;
        }

        inline bool equal(const C c, const C c2, bool case_sensitive = false, bool accent_sensitive = false) noexcept
        {
            if (case_sensitive)
            {
                if (accent_sensitive)
                {
                    return c == c2;
                }
                else
                {
                    return copy_remove_accent(c) == copy_remove_accent(c2);
                }
            }
            else
            {
                if (accent_sensitive)
                {
                    return std::tolower(c) == std::tolower(c2);
                }
                else
                {
                    return std::tolower(copy_remove_accent(c)) == std::tolower(copy_remove_accent(c2));
                }
            }
        }

        inline bool less(const C c, const C c2, bool case_sensitive = false, bool accent_sensitive = false) noexcept
        {
            if (case_sensitive)
            {
                if (accent_sensitive)
                {
                    return c < c2;
                }
                else
                {
                    return copy_remove_accent(c) < copy_remove_accent(c2);
                }
            }
            else
            {
                if (accent_sensitive)
                {
                    return std::tolower(c) < std::tolower(c2);
                }
                else
                {
                    return std::tolower(copy_remove_accent(c)) < std::tolower(copy_remove_accent(c2));
                }
            }
        }

        inline bool not_equal(const C c, const C c2, bool case_sensitive = false, bool accent_sensitive = false) noexcept
        {
            return !equal(c, c2, case_sensitive, accent_sensitive);
        }

        template <typename C = char>
        inline bool greater(const C c, const C c2, bool case_sensitive = false, bool accent_sensitive = false) noexcept
        {
            return !less(c, c2, case_sensitive, accent_sensitive) && !equal(c, c2, case_sensitive, accent_sensitive);
        }

        inline bool less_equal(const C c, const C c2, bool case_sensitive = false, bool accent_sensitive = false) noexcept
        {
            return less(c, c2, case_sensitive, accent_sensitive) || equal(c, c2, case_sensitive, accent_sensitive);
        }

        inline bool greater_equal(const C c, const C c2, bool case_sensitive = false, bool accent_sensitive = false) noexcept
        {
            return greater(c, c2, case_sensitive, accent_sensitive) || equal(c, c2, case_sensitive, accent_sensitive);
        }


        template<size_t MIN = 0, size_t MAX = 20, bool CaseSensitive = false, bool AccentSensitive = false>
        class CS
        {
        public:
            using value_type = C;
            std::array<C, MAX> data;
            inline static constexpr bool case_sensitive = CaseSensitive;
            inline static constexpr bool accent_sensitive = AccentSensitive;
            inline static const size_t MAX_SIZE = MAX;
            inline static const size_t MAX_LENGTH = MAX - 1;
            inline static const size_t MIN_SIZE = MIN;
   
            // Returns the size of the string.
            const constexpr inline size_t size() const noexcept
            {
                return MAX;
            }

            void inline fill(C c) noexcept
            {
                data.fill(c);
            }

            void fill_null() noexcept
            {
                fill(NULL_CHAR);
            }

            // Default constructor
            CS()
            {
                fill_null();
            }

            inline bool is_null_char(size_t index) const noexcept
            {
                return (data[index] == NULL_CHAR);
            }

            inline size_t length() const noexcept
            {
                return std::char_traits<C>::length(data.data());
            }

            inline void copy(const C* s, size_t s_length, bool add_null_at_end = true, bool fill_null_before_copy = true)
            {
                if (s_length >= MAX)
                {
                    std::string error = "String is too long. Max size is ";
                    error += std::to_string(MAX);
                    throw std::runtime_error(error);
                }
                if (s_length < MIN)
                {
                    std::string error = "String is too short. Min size is ";
                    error += std::to_string(MIN);
                    throw std::runtime_error(error);
                }
                if (fill_null_before_copy)
                    fill_null();

                if (s_length > 0)
                    std::memcpy(data.data(), s, s_length * sizeof(C));

                if (add_null_at_end and (s_length >= 0))
                    data[s_length] = NULL_CHAR;
            }

            inline void copy(const C* str)
            {
                auto strlen = std::char_traits<C>::length(str);
                copy(str, strlen);
            }

            inline void copy(const S& str)
            {
                copy(str.c_str(), str.length());
            }

            CS(const C* str)
            {
                copy(str);
            }

            CS(const S& str)
            {
                copy(str.c_str(), str.length());
            }

            // Converts to C*. Must be null terminated.
            operator C* () const noexcept
            {
                // Allocate memory for the new string.
                size_t size = length() + 1;
                C* c = new C[size];
                // Copy the string. With null termination.
                std::memcpy(c, data.data(), size);
                return c;
            }

            inline S to_string() const noexcept
            {
                return S(data.data());
            }

            inline S str() const noexcept
            {
                return S(data.data());
            }

            // Converts to S.
            operator S() const noexcept
            {
                return to_string();
            }

            // Compare strings length.
            inline size_t cmp_strlen(const CS& other) const noexcept
            {
                return length() - other.length();
            }

            inline bool eq_strlen(const CS& other) const noexcept
            {
                return length() == other.length();
            }

            inline bool empty() const noexcept
            {
                return length() == 0;
            }

            // operator[]
            inline C& operator[] (const size_t index) const noexcept
            {
                // Removes const and returns C&.
                return const_cast<C&>(data[index]);
            }

            inline C& at(const size_t index) const noexcept
            {
                return operator[](index);
            }

            // Comparison operators
            bool operator== (const CS& other) const noexcept
            {
                bool result = eq_strlen(other);
                if (result)
                {
                    auto strlen = length();
                    for (size_t i = 0; i < strlen; ++i)
                    {
                        if (!equal(data[i], other.data[i], case_sensitive, accent_sensitive))
                        {
                            result = false;
                            break;
                        }
                    }
                }

                return result;
            }

            bool operator!=(const CS& other) const noexcept
            {
                return !(*this == other);
            }

            bool operator<(const CS& other) const noexcept
            {
                bool result = less(data[0], other.data[0], case_sensitive, accent_sensitive);
                if (result)
                {
                    auto strlen = length();
                    for (size_t i = 1; i < strlen; ++i)
                    {
                        if (!less(data[i], other.data[i], case_sensitive, accent_sensitive))
                        {
                            result = false;
                            break;
                        }
                    }
                }

                return result;
            }

            bool operator>(const CS& other) const noexcept
            {
                return other < *this;
            }

            bool operator<=(const CS& other) const noexcept
            {
                return !(other < *this);
            }

            bool operator>=(const CS& other) const noexcept
            {
                return !(*this < other);
            }

            // Assigns a std::basic_string.
            CS& operator= (const S& str) noexcept
            {
                copy(str);
                return *this;
            }

            // Assigns a null terminated string.
            CS& operator= (const C* str)
            {
                copy(str);
                return *this;
            }

            // Assigns a std::array.
            CS& operator= (const std::array<C, MAX>& arr) noexcept
            {
                std::memcpy(data.data(), arr.data(), MAX * sizeof(C));
                return *this;
            }

            CS& assign(const CS& other)
            {
                copy(other.data.data(), other.length(), ADD_NULL_AT_END);
                return *this;
            }

            CS& operator+= (const CS& other)
            {
                auto strlen = length();
                auto other_strlen = other.length();
                if (strlen + other_strlen > MAX)
                {
                    std::string error = "CString is too long. Max size is ";
                    error += std::to_string(MAX);
                    throw std::runtime_error(error);
                }
                std::memcpy(data.data() + strlen, other.data.data(), other_strlen * sizeof(C));
                data[strlen + other_strlen] = NULL_CHAR;
                return *this;
            }

            CS operator+ (const CS& other)
            {
                CS result = *this;
                result += other;
                return result;
            }

            // Makes CS compatible with NarrowOutuputStreamable concept.
            OutStream& write(OutStream& os) const noexcept
            {
                os << data.data();
                return os;
            }

            InStream& read(InStream& is)
            {
                S s;
                is >> s;
                copy(s);
                return is;
            }
        };  // class CS

 


        template <size_t MIN = 0, size_t MAX>
        OutStream& operator<< (OutStream& os, const CS<MIN, MAX>& cs)
        {
            return cs.write(os);
        }

        template <size_t MIN = 0, size_t MAX>
        InStream& operator>> (InStream& is, CS<MIN, MAX>& cs)
        {
            return cs.read(is);
        }


        // Concatenates two CS objects. Must be of same char type.
        template<int N, int N2>
        CS<0, N + N2> operator+ (const CS<0, N>& lhs, const CS<0, N2>& rhs) noexcept
        {
            static_assert (N > 0, W("CS<N> operator+ (const CS<N>& lhs, const CS<N>& rhs) - lhs must be of size N > 0"));

            CS<0, N + N2> result;
            std::copy(lhs.data.begin(), lhs.data.end(), result.data.begin());
            std::copy(rhs.data.begin(), rhs.data.end(), result.data.begin() + N);
            return result;
        }

        // Concatenates a CS object and a std::basic_string. Must be of same char type.
        template<int N, int N2, typename Char = char>
        CS<0, N + N2> operator+ (const CS<0, N>& lhs, const S& rhs)
        {
            static_assert (N > 0, W("CS<N> operator+ (const CS<N>& lhs, const S& rhs) - lhs must be of size N > 0"));

            if (rhs.size() != N2)
            {
                throw std::runtime_error(W("CS<N> operator+ (const CS<N>& lhs, const std::basic_string<Char>& rhs) - rhs must be of size N2"));
            }

            CS<0, N + N2> result;
            std::copy(lhs.data.begin(), lhs.data.end(), result.data.begin());
            std::copy(rhs.begin(), rhs.end(), result.data.begin() + N);
            return result;
        }

        // Concatenates a CS object and a null terminated string. Must be of same char type.
        template<int N, int N2>
        CS<0, N + N2> operator+ (const CS<0, N>& lhs, const C* rhs)
        {
            static_assert (N > 0, W("CS<N> operator+ (const CS<N>& lhs, const Char* rhs) - lhs must be of size N > 0"));

            if (std::char_traits<C>::length(rhs) != N2)
            {
                throw std::runtime_error(W("CS<N> operator+ (const CS<N>& lhs, const Char* rhs) - rhs must be of size N2"));
            }
            CS<0, N + N2> result;
            std::copy(lhs.data.begin(), lhs.data.end(), result.data.begin());
            std::copy(rhs, rhs + N, result.data.begin() + N);
            return result;
        }

        // Concatenates a CS object and a std::array. Must be of same char type.
        template<int N, size_t N2>
        CS<0, N + N2> operator+ (const CS<0, N>& lhs, const std::array<C, N2>& rhs) noexcept
        {
            static_assert (N2 > 0, W("CS<N> operator+ (const CS<N>& lhs, const std::array<Char, N2>& rhs) - rhs must be of size > 0"));
            static_assert (N > 0, W("CS<N> operator+ (const CS<N>& lhs, const std::array<Char, N2>& rhs) - lhs must be of size > 0"));

            CS<0, N + N2> result;
            std::copy(lhs.data.begin(), lhs.data.end(), result.data.begin());
            std::copy(rhs.begin(), rhs.end(), result.data.begin() + N);
            return result;
        }

        // Concatenates an S string and a CS object. Must be of same char type.
        template<size_t N>
        CS<0, N + 1> operator+ (const S& lhs, const CS<0, N>& rhs)
        {
            size_t min_size = lhs.length() + rhs.length() + 1;
            if (N < min_size)
                throw std::runtime_error("CS<N> operator+ (const std::basic_string<Char>& lhs, const CS<N>& rhs) - rhs must be of size > " + std::to_string(min_size));
            CS<0, N + 1> result;
            std::copy(lhs.begin(), lhs.end(), result.data.begin());
            std::copy(rhs.data.begin(), rhs.data.begin() + rhs.length(), result.data.begin() + lhs.length());
            return result;
        }

        // Concatenates a std::array and a CS object. Must be of same char type.
        template<int N, int N2>
        CS<0, N + N2> operator+ (const std::array<C, N>& lhs, const CS<0, N2>& rhs) noexcept
        {
            static_assert (N > 0, W("CS<N> operator+ (const std::array<Char, N>& lhs, const CS<N2>& rhs) - lhs must be of size > 0"));

            CS<0, N + N2> result;
            std::copy(lhs.begin(), lhs.end(), result.data.begin());
            std::copy(rhs.data.begin(), rhs.data.end(), result.data.begin() + N);
            return result;
        }

        // Concatenates a CS object and a char. Must be of same char type.
        template<int N>
        CS<0, N + sizeof(C)> operator+ (const CS<0, N>& lhs, const C& rhs) noexcept
        {
            static_assert (N > 0, W("CS<N> operator+ (const CS<N>& lhs, const Char& rhs) - lhs must be of size > 0"));

            CS<0, N + sizeof(C)> result;
            std::copy(lhs.data.begin(), lhs.data.end(), result.data.begin());
            result.data[N] = rhs;
            return result;
        }

        // Concatenates a char and a CS object. Must be of same char type.
        template<int N>
        CS<0, N + sizeof(C)> operator+ (const C& lhs, const CS<0, N>& rhs) noexcept
        {
            static_assert (N > 0, W("CS<N> operator+ (const Char& lhs, const CS<N>& rhs) - lhs must be of size > 0"));

            CS<0, N + sizeof(C)> result;
            result.data[0] = lhs;
            std::copy(rhs.data.begin(), rhs.data.end(), result.data.begin() + 1);
            return result;
        }

        // Assignment operator for std::array<C, N> with other std::array. returns a std::array<C, N>&.
        template<int N>
        std::array<C, N>& assign(std::array<C, N>& lhs, const std::array<C, N>& rhs) noexcept
        {
            std::memcpy(lhs.data(), rhs.data(), N);
            return lhs;
        }

    }
}
#endif // CS_HPP

