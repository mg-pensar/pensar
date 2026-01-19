// author : Mauricio Gomes
// license: MIT (https://opensource.org/licenses/MIT)

#ifndef ARRAY_HPP
#define ARRAY_HPP

#include "concept.hpp"

#include "equal.hpp"
#include "s.hpp"

#include "code_util.hpp"

#include <limits>



namespace pensar_digital
{
    namespace cpplib
    {
        template <typename T>
        class Array
        {
        private:
            T* marray;
            size_t msize;
        public:
            // Define value_type as T.
            typedef T value_type;

            Array(size_t size) : msize(size) { marray = new T[size]; }
            ~Array() { delete[] marray; }

            //T& operator[] (const size_t index) const { return marray[index]; }
            //T& at (const size_t index) const { return marray[index]; }
            constexpr T& operator[] (const size_t index) const { return marray[index]; }
            constexpr T& at(const size_t index) const { return marray[index]; }

            [[nodiscard]] constexpr T* data() const noexcept { return marray; }

            [[nodiscard]] constexpr size_t size() const noexcept { return msize; }
        };

		// A StdLayoutTriviallyCopyable version of Array.
        // Elements must be comparable via std::memcmp.
        template <size_t Size = 1, typename T = size_t>
            requires StdLayoutTriviallyCopyable<T>
        struct CArray 
        {
            T _array[Size] = {};  // Zero-initialized, no padding

            using value_type = T;
            static constexpr size_t SIZE = Size;

            // Default constructor
            constexpr CArray() noexcept = default;

            // Initialize from initializer_list
            constexpr CArray(std::initializer_list<T> init) noexcept : _array{} {
                std::copy_n(init.begin(), (std::min)(init.size(), Size), _array);
            }

            // Variadic constructor for direct initialization
            template<typename... Args>
                requires (sizeof...(Args) <= Size) && (std::convertible_to<Args, T> && ...)
            constexpr CArray(Args... args) noexcept : _array{ static_cast<T>(args)... } {}

            // Access operators
            constexpr T& operator[](size_t index) noexcept {
                return _array[index];
            }

            constexpr const T& operator[](size_t index) const noexcept {
                return _array[index];
            }

            // Bounds-checked access
            constexpr T& at(size_t index) 
            {
                if (index >= SIZE) 
                {
                    throw std::out_of_range(W("CArray::at: index out of bounds"));
                }
                return _array[index];
            }

            constexpr const T& at(size_t index) const 
            {
                if (index >= SIZE) 
                {
                    throw std::out_of_range(W("CArray::at: index out of bounds"));
                }
                return _array[index];
            }

            constexpr T* data() noexcept { return _array; }
            constexpr const T* data() const noexcept { return _array; }
            constexpr size_t size() const noexcept { return SIZE; }

            // Full comparison using memcmp (safe - no padding)
            constexpr bool operator==(const CArray& other) const noexcept {
                return std::memcmp(_array, other._array, sizeof(_array)) == 0;
            }

            constexpr bool operator!=(const CArray& other) const noexcept {
                return !(*this == other);
            }

            // Compare only first 'count' elements
            constexpr bool compare_range(const CArray& other, size_t count) const noexcept {
                if (count > SIZE) count = SIZE;
                return std::memcmp(_array, other._array, count * sizeof(T)) == 0;
            }

            // Compare a specific range [start, start+count)
            constexpr bool compare_range(const CArray& other, size_t start, size_t count) const noexcept {
                if (start >= SIZE) return false;
                if (start + count > SIZE) return false;
                return std::memcmp(_array + start, other._array + start, count * sizeof(T)) == 0;
            }

            // Compare against raw pointer (useful for BOM checking)
            constexpr bool compare_range(const T* ptr, size_t count) const noexcept {
                if (count > SIZE) count = SIZE;
                return std::memcmp(_array, ptr, count * sizeof(T)) == 0;
            }

			// Check if element is present in array.
            constexpr Result<size_t> contains(const T& element) const noexcept
            {
                for (size_t i = 0; i < SIZE; ++i) {
                    if (_array[i] == element) {
                        return { i, true };
                    }
                }
                return {0xffffffff , false, W("Element not found in array.")};
			}

            // Check if starts with specific pattern
            constexpr bool starts_with(const CArray& pattern, size_t pattern_len) const noexcept {
                return compare_range(pattern, pattern_len);
            }

            // Check if starts with initializer list
            constexpr bool starts_with(std::initializer_list<T> pattern) const noexcept {
                if (pattern.size() > SIZE) return false;
                return std::memcmp(_array, pattern.begin(), pattern.size() * sizeof(T)) == 0;
            }

            // Fill entire array with value
            constexpr void fill(T value) noexcept {
                std::fill_n(_array, SIZE, value);
            }

            // Fill first 'count' elements with value
            constexpr void fill(T value, size_t count) noexcept {
                if (count > SIZE) count = SIZE;
                std::fill_n(_array, count, value);
            }

            // Zero out entire array
            constexpr void clear() noexcept {
                std::memset(_array, 0, sizeof(_array));
            }

            // Iterator support (for range-based for loops)
            constexpr T* begin() noexcept { return _array; }
            constexpr const T* begin() const noexcept { return _array; }
            constexpr T* end() noexcept { return _array + SIZE; }
            constexpr const T* end() const noexcept { return _array + SIZE; }
        };
    }
}
#endif // ARRAY_HPP
