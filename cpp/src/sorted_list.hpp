// author: Mauricio Gomes
// license: MIT[](https://opensource.org/licenses/MIT)

#ifndef SORTED_LIST_HPP
#define SORTED_LIST_HPP

#include "string_def.hpp"
#include <vector>
#include <algorithm>
#include <ranges>
#include <iterator>
#include <stdexcept>
#include <concepts>
#include <functional>

#include "code_util.hpp"

namespace pensar_digital
{
    namespace cpplib
    {
        // Concept to ensure type is comparable with a given comparator
        template<typename T, typename Compare>
        concept ComparableWith = requires(T a, T b, Compare comp)
        {
            { comp(a, b) } -> std::convertible_to<bool>;
            { a == b } -> std::convertible_to<bool>;
        };

        // Validate index function
        inline Result<bool> validate_index(size_t index, size_t size, const S& context = W("SortedList"), bool throws = true)
        {
            if (index >= size)
            {
                if (throws)
                    throw std::out_of_range(context + W(": Index ") + std::to_string(index) +
						W(" out of bounds for size ") + std::to_string(size));
                return { context + W(": Index ") + std::to_string(index) +
                    W(" out of bounds for size ") + std::to_string(size), false };
            }
            return true;
        }

        template<typename T, typename Compare = std::less<T>>
            requires ComparableWith<T, Compare>&& std::strict_weak_order<Compare, T, T>
        class SortedList
        {
        public:
            using value_type = T;
            using iterator = typename std::vector<T>::iterator;
            using const_iterator = typename std::vector<T>::const_iterator;
            using reverse_iterator = typename std::vector<T>::reverse_iterator;
            using const_reverse_iterator = typename std::vector<T>::const_reverse_iterator;
            using comparator_type = Compare;

            // Constructors
            SortedList(bool unique = false, const Compare& comp = Compare())
                : unique_(unique), comp_(comp) {
            }

            // Initializer list constructor
            SortedList(std::initializer_list<T> init, bool unique = false, const Compare& comp = Compare())
                : unique_(unique), comp_(comp)
            {
                data_.reserve(init.size());
                for (const auto& item : init)
                {
                    add(item);
                }
            }

            // Range constructor
            template<std::ranges::input_range R>
            SortedList(R&& range, bool unique = false, const Compare& comp = Compare())
                : unique_(unique), comp_(comp)
            {
                for (const auto& item : range)
                {
                    add(item);
                }
            }

            // Copy constructor
            SortedList(const SortedList& other) = default;

            // Move constructor
            SortedList(SortedList&& other) noexcept = default;

            // Assignment operators
            SortedList& operator=(const SortedList& other) = default;
            SortedList& operator=(SortedList&& other) noexcept = default;

            // Destructor
            ~SortedList() = default;

            // Conversion operator to S
            operator S() const { return to_s(); }

            // String representation of the list
            S to_s() const
            {
                SStream ss;
                ss << W("[");
                bool first = true;
                for (const auto& item : *this)
                {
                    if (!first)
                        ss << W(", ");
                    ss << item;
                    first = false;
                }
                ss << W("]");
                return ss.str();
            }

            // Set uniqueness
            void set_unique(bool unique) noexcept { unique_ = unique; }

            // Get uniqueness
            bool is_unique() const noexcept { return unique_; }

            // Add element in sorted order, returns true if added, false if rejected due to uniqueness
            bool add(const T& value)
            {
                try
                {
                    auto it = std::lower_bound(data_.begin(), data_.end(), value, comp_);
                    if (unique_ && it != data_.end() && !comp_(*it, value) && !comp_(value, *it))
                    {
                        return false; // Item already exists in unique list
                    }
                    data_.insert(it, value);
                    return true;
                }
                catch (const std::exception& e)
                {
                    throw std::runtime_error(W("Failed to add element: ") + S(e.what()));
                }
            }

            // Find element, returns const_iterator to element or end() if not found
            const_iterator find(const T& value) const
            {
                try
                {
                    auto it = std::lower_bound(data_.begin(), data_.end(), value, comp_);
                    if (it != data_.end() && !comp_(*it, value) && !comp_(value, *it))
                        return it;
                    return data_.end();
                }
                catch (const std::exception& e)
                {
                    throw std::runtime_error(W("Failed to find element: ") + S(e.what()));
                }
            }
            // Remove element, returns true if removed
            bool remove(const T& value)
            {
                try
                {
                    auto it = find(value);
                    if (it != data_.end())
                    {
                        data_.erase(it);
                        return true;
                    }
                    return false;
                }
                catch (const std::exception& e)
                {
                    throw std::runtime_error(W("Failed to remove element: ") + S(e.what()));
                }
            }

            // Remove element at index
            void remove_at(size_t index)
            {
                validate_index(index, data_.size(), W("SortedList::remove_at"));
                data_.erase(data_.begin() + index);
            }

            // Access element at index (const and non-const)
            T& at(size_t index)
            {
                validate_index(index, data_.size(), W("SortedList::at"));
                return data_[index];
            }

            const T& at(size_t index) const
            {
                validate_index(index, data_.size(), W("SortedList::at"));
                return data_[index];
            }

            // Subscript operator (non-const)
            T& operator[](size_t index)
            {
                validate_index(index, data_.size(), W("SortedList::operator[]"));
                return data_[index];
            }

            // Subscript operator (const)
            const T& operator[](size_t index) const
            {
                validate_index(index, data_.size(), W("SortedList::operator[]"));
                return data_[index];
            }

            // Check if list contains value
            bool contains(const T& value) const
            {
                return find(value) != data_.end();
            }

            // Get size of list
            size_t size() const noexcept { return data_.size(); }

            // Check if list is empty
            bool empty() const noexcept { return data_.empty(); }

            // Clear all elements
            void clear() noexcept { data_.clear(); }

            // Iterator support
            iterator begin() noexcept { return data_.begin(); }
            const_iterator begin() const noexcept { return data_.begin(); }
            const_iterator cbegin() const noexcept { return data_.cbegin(); }
            iterator end() noexcept { return data_.end(); }
            const_iterator end() const noexcept { return data_.end(); }
            const_iterator cend() const noexcept { return data_.cend(); }

            // Reverse iterator support
            reverse_iterator rbegin() noexcept { return data_.rbegin(); }
            const_reverse_iterator rbegin() const noexcept { return data_.rbegin(); }
            const_reverse_iterator crbegin() const noexcept { return data_.crbegin(); }
            reverse_iterator rend() noexcept { return data_.rend(); }
            const_reverse_iterator rend() const noexcept { return data_.rend(); }
            const_reverse_iterator crend() const noexcept { return data_.crend(); }

            // Range support
            auto as_range() const noexcept { return std::ranges::subrange(data_.begin(), data_.end()); }

            // Get underlying container (for advanced use)
            const std::vector<T>& data() const noexcept { return data_; }

            // Get comparator
            const Compare& comparator() const noexcept { return comp_; }

			// An alias for back() with a name that makes much more sense.
            inline auto& last () const
            {
                if (data_.empty())
                    throw std::out_of_range(W("SortedList::back: List is empty"));
                return data_.back();
			}

			// Gets the last element in the list.
            inline auto& back() const
            {
                return last();
			}


        private:
            std::vector<T> data_;
            bool unique_ = false; // Default to allowing duplicates
            Compare comp_;
        };
    }
}

#endif // SORTED_LIST_HPP