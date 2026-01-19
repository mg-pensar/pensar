#ifndef EQUAL_HPP
#define EQUAL_HPP

#include "concept.hpp"
#include "constant.hpp"
#include <type_traits>
#include <cstring>

namespace pensar_digital
{
    namespace cpplib
    {
        // TriviallyHashComparable concept.
        template <class T>
        concept TriviallyHashComparable = HasStdLayoutTriviallyCopyableData<T> && pensar_digital::cpplib::Hashable<T>;

        // Tag types for dispatch.
        struct HashComparableTag {};
        struct DataComparableTag {};
		struct TriviallyComparableTag {};
		struct StdLayoutTriviallyCopyableTag {};

        // Type trait to select the appropriate tag.
        template <class T>
        using EqualDispatchTag = std::conditional_t<
            pensar_digital::cpplib::TriviallyHashComparable<T>,
            pensar_digital::cpplib::HashComparableTag,
            std::conditional_t<
                pensar_digital::cpplib::HasStdLayoutTriviallyCopyableData<T>,
                pensar_digital::cpplib::DataComparableTag,
                std::conditional_t<
                    std::is_trivially_copyable_v<T>,
                    pensar_digital::cpplib::StdLayoutTriviallyCopyableTag,
                    pensar_digital::cpplib::TriviallyComparableTag
                >
            >
        >;

        // Implementation for TriviallyHashComparable (uses hash).
        template <TriviallyHashComparable T>
        bool equal_impl(const T& l, const T& r, HashComparableTag)
        {
            if (l.hash() != r.hash())
                return false;
            return equal_impl(l, r, DataComparableTag{}); // Fall back to memcmp
        }

        // Implementation for HasStdLayoutTriviallyCopyableData (no hash).
        template <HasStdLayoutTriviallyCopyableData T>
        bool equal_impl(const T& l, const T& r, DataComparableTag)
        {
            if (&l == &r) // Same object
                return true;
            return std::memcmp(l.data(), r.data(), l.data_size()) == 0;
        }

		// Implementation for TriviallyComparable.
        bool equal_impl(const auto& l, const auto& r, TriviallyComparableTag)
        {
            return l == r;
		}
        // Implementation for StdLayoutTriviallyCopyable.
        template <StdLayoutTriviallyCopyable T>
        bool equal_impl(const T& l, const T& r, StdLayoutTriviallyCopyableTag)
        {
            if (&l == &r) // Same object
                return true;
            return std::memcmp(&l, &r, sizeof(l)) == 0;
        }

        // Public interface: single equal function.
        template <typename T>
        bool equal(const T& l, const T& r)
        {
            return equal_impl(l, r, EqualDispatchTag<T>{});
        }
    }
}
#endif // EQUAL_HPP
