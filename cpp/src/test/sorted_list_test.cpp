// author: Mauricio Gomes
// license: MIT[](https://opensource.org/licenses/MIT)

#include "../../../unit_test/src/test.hpp"
#include "../sorted_list.hpp"
#include <cwctype> // For std::towlower
#include <clocale> // For std::setlocale

namespace pensar_digital
{
    namespace test = pensar_digital::unit_test;
    using namespace pensar_digital::unit_test;
    namespace cpplib
    {
        // Custom comparator for descending order (integers)
        struct Descending {
            bool operator()(size_t a, size_t b) const { return a > b; }
        };

        // Custom comparator for case-insensitive string comparison
        struct CaseInsensitive {
            bool operator()(const S& a, const S& b) const {
                S a_lower = a;
                S b_lower = b;
#ifdef WIDE_CHAR
                std::transform(a_lower.begin(), a_lower.end(), a_lower.begin(),
                    [](C c) { return std::towlower(c); });
                std::transform(b_lower.begin(), b_lower.end(), b_lower.begin(),
                    [](C c) { return std::towlower(c); });
#else
                std::transform(a_lower.begin(), a_lower.end(), a_lower.begin(),
                    [](C c) { return std::tolower(c); });
                std::transform(b_lower.begin(), b_lower.end(), b_lower.begin(),
                    [](C c) { return std::tolower(c); });
#endif
                return a_lower < b_lower;
            }
        };

        TEST(SortedList, true)
            // Set locale for consistent case conversion
            std::setlocale(LC_ALL, "C");

        // Basic functionality with default comparator (ascending)
        SortedList<size_t > sl = {};
        CHECK_EQ(size_t , sl.size(), 0, W("0: Default constructor size"));
        CHECK_EQ(bool, sl.is_unique(), false, W("1: Default constructor not unique"));

        sl.add(2);
        CHECK_EQ(size_t, sl.size(), 1, W("2: Size after adding one element"));
        CHECK_EQ(size_t, sl[0], 2, W("3: Element at index 0"));

        sl.add(0);
        CHECK_EQ(size_t, sl.size(), 2, W("4: Size after adding second element"));
        CHECK_EQ(size_t, sl[0], 0, W("5: Element at index 0 after sorting"));
        CHECK_EQ(size_t, sl[1], 2, W("6: Element at index 1 after sorting"));

        sl.add(1);
        CHECK_EQ(size_t, sl.size(), 3, W("7: Size after adding third element"));
        CHECK_EQ(size_t, sl[1], 1, W("8: Element at index 1 after sorting"));

        // Test custom comparator (descending)
        SortedList<size_t, Descending> desc_sl({ 3, 1, 4, 1, 5 }, false, Descending{});
        CHECK_EQ(size_t, desc_sl.size(), 5, W("9: Descending list size"));
        CHECK_EQ(size_t, desc_sl[0], 5, W("10: Descending list first element"));
        CHECK_EQ(size_t, desc_sl[1], 4, W("11: Descending list second element"));
        CHECK_EQ(size_t, desc_sl[2], 3, W("12: Descending list third element"));
        CHECK_EQ(size_t, desc_sl[3], 1, W("13: Descending list fourth element"));
        CHECK_EQ(size_t, desc_sl[4], 1, W("14: Descending list fifth element"));

        // Test add with custom comparator
        CHECK_EQ(bool, desc_sl.add(2), true, W("15: Add 2 to descending list"));
        CHECK_EQ(size_t, desc_sl.size(), 6, W("16: Size after adding to descending list"));
        CHECK_EQ(size_t, desc_sl[3], 2, W("17: Element at index 3 after adding 2"));

        // Test find
        auto it = sl.find(1);
        CHECK_EQ(size_t, it != sl.end() ? *it : -1, 1, W("18: Find existing element"));
        it = sl.find(99);
        CHECK_EQ(bool, it == sl.end(), true, W("19: Find non-existing element"));

        // Test contains
        CHECK_EQ(bool, sl.contains(2), true, W("20: Contains existing element"));
        CHECK_EQ(bool, sl.contains(99), false, W("21: Contains non-existing element"));

        // Test remove
        CHECK_EQ(bool, sl.remove(1), true, W("22: Remove existing element"));
        CHECK_EQ(size_t, sl.size(), 2, W("23: Size after remove"));
        CHECK_EQ(bool, sl.contains(1), false, W("24: Element removed"));
        CHECK_EQ(bool, sl.remove(99), false, W("25: Remove non-existing element"));

        // Test remove_at
        sl.remove_at(0);
        CHECK_EQ(size_t, sl.size(), 1, W("26: Size after remove_at"));
        CHECK_EQ(size_t, sl[0], 2, W("27: Element after remove_at"));

        // Test iterator
        int expected[] = { 2 };
        int i = 0;
        for (const auto& item : sl) {
            CHECK_EQ(size_t, item, expected[i], W("28: Iterator element at index ") + std::to_string(i));
            ++i;
        }
        CHECK_EQ(size_t, i, 1, W("29: Iterator count"));

        // Test range
        i = 0;
        for (const auto& item : sl.as_range()) {
            CHECK_EQ(size_t, item, expected[i], W("30: Range element at index ") + std::to_string(i));
            ++i;
        }
        CHECK_EQ(size_t, i, 1, W("31: Range count"));

        // Test clear
        sl.clear();
        CHECK_EQ(size_t, sl.size(), 0, W("32: Size after clear"));
        CHECK_EQ(bool, sl.empty(), true, W("33: Empty after clear"));

        // Test error handling for operator[]
        bool exception_thrown = false;
        try {
            sl[0];
        }
        catch (const std::out_of_range&) {
            exception_thrown = true;
        }
        CHECK_EQ(bool, exception_thrown, true, W("34: operator[] throws on empty list"));

        // Test error handling for at
        exception_thrown = false;
        try {
            sl.at(0);
        }
        catch (const std::out_of_range&) {
            exception_thrown = true;
        }
        CHECK_EQ(bool, exception_thrown, true, W("35: at throws on empty list"));

        // Test reverse iterator
        desc_sl.clear();
        desc_sl.add(3);
        desc_sl.add(2);
        desc_sl.add(1);
        int rev_expected[] = { 1, 2, 3 };
        i = 0;
        for (auto it = desc_sl.rbegin(); it != desc_sl.rend(); ++it) {
            CHECK_EQ(size_t, *it, rev_expected[i], W("36: Reverse iterator element at index ") + std::to_string(i));
            ++i;
        }
        CHECK_EQ(size_t, i, 3, W("37: Reverse iterator count"));

        // Test comparator access
        CHECK_EQ(bool, desc_sl.comparator()(5, 3), true, W("38: Comparator check (5 > 3)"));

        // Test operator[] with custom comparator
        CHECK_EQ(size_t, desc_sl[0], 3, W("39: Descending list operator[] at index 0"));
        CHECK_EQ(size_t, desc_sl[1], 2, W("40: Descending list operator[] at index 1"));
        CHECK_EQ(size_t, desc_sl[2], 1, W("41: Descending list operator[] at index 2"));

        // Test sorted order after multiple adds
        SortedList<size_t> asc_sl2 = {};
        asc_sl2.add(5);
        asc_sl2.add(2);
        asc_sl2.add(8);
        asc_sl2.add(1);
        size_t asc_expected[] = { 1, 2, 5, 8 };
        i = 0;
        for (const auto& item : asc_sl2) {
            CHECK_EQ(size_t, item, asc_expected[i], W("42: Ascending order after multiple adds at index ") + std::to_string(i));
            ++i;
        }
        CHECK_EQ(size_t, i, 4, W("43: Ascending order count"));

        // Test case-insensitive string comparator
        SortedList<S, CaseInsensitive> str_sl({ W("Apple"), W("banana"), W("date") }, false, CaseInsensitive{});
        CHECK_EQ(size_t, str_sl.size(), 3, W("44: String list size"));
        CHECK_EQ(S, str_sl[0], W("Apple"), W("45: String list first element (case-insensitive)"));
        CHECK_EQ(S, str_sl[1], W("banana"), W("46: String list second element (case-insensitive)"));
        CHECK_EQ(S, str_sl[2], W("date"), W("47: String list third element (case-insensitive)"));
        CHECK_EQ(bool, str_sl.add(W("Cherry")), true, W("48: Add 'Cherry' to string list"));
        CHECK_EQ(S, str_sl[3], W("date"), W("49: String list after adding 'date'"));
        CHECK_EQ(S, str_sl.to_s(), W("[Apple, banana, Cherry, date]"), W("50: to_s after adding 'Cherry'"));

        // Test to_s and conversion to S
        CHECK_EQ(S, sl.to_s(), W("[]"), W("51: to_s on empty list"));
        CHECK_EQ(S, S(sl), W("[]"), W("52: Conversion to S on empty list"));
        CHECK_EQ(S, asc_sl2.to_s(), W("[1, 2, 5, 8]"), W("53: to_s on ascending list"));
        CHECK_EQ(S, S(asc_sl2), W("[1, 2, 5, 8]"), W("54: Conversion to S on ascending list"));
        CHECK_EQ(S, desc_sl.to_s(), W("[3, 2, 1]"), W("55: to_s on descending list"));
        CHECK_EQ(S, S(desc_sl), W("[3, 2, 1]"), W("56: Conversion to S on descending list"));

        // Test copy constructor
        SortedList<size_t, Descending> desc_sl_copy = desc_sl;
        CHECK_EQ(size_t, desc_sl_copy.size(), 3, W("57: Copy constructor size"));
        CHECK_EQ(size_t, desc_sl_copy[0], 3, W("58: Copy constructor first element"));
        CHECK_EQ(bool, desc_sl_copy.comparator()(5, 3), true, W("59: Copy constructor comparator"));

        // Test move constructor
        SortedList<size_t, Descending> desc_sl_move = std::move(desc_sl_copy);
        CHECK_EQ(size_t, desc_sl_move.size(), 3, W("60: Move constructor size"));
        CHECK_EQ(size_t, desc_sl_move[0], 3, W("61: Move constructor first element"));
        CHECK_EQ(size_t, desc_sl_copy.size(), 0, W("62: Moved-from list size"));

        // Test empty iterator
        i = 0;
        for (const auto& item : sl) {
            i++;
        }
        CHECK_EQ(size_t, i, 0, W("63: Empty iterator count"));

        // Test adding duplicates with custom comparator
        CHECK_EQ(bool, desc_sl.add(2), true, W("64: Add duplicate to non-unique descending list"));
        CHECK_EQ(size_t, desc_sl.size(), 4, W("65: Size after adding duplicate"));
        CHECK_EQ(size_t, desc_sl[1], 2, W("66: First duplicate at index 1"));
        CHECK_EQ(size_t, desc_sl[2], 2, W("67: Second duplicate at index 2"));

        // Test unique list with default comparator
        SortedList<size_t> unique_sl ({}, true);
        CHECK_EQ(bool, unique_sl.is_unique(), true, W("68: Unique constructor"));
        CHECK_EQ(bool, unique_sl.add(1), true, W("69: Add to unique list"));
        CHECK_EQ(size_t, unique_sl.size(), 1, W("70: Size after first add to unique list"));
        CHECK_EQ(bool, unique_sl.add(1), false, W("71: Reject duplicate in unique list"));
        CHECK_EQ(size_t, unique_sl.size(), 1, W("72: Size unchanged after rejected duplicate"));
        unique_sl.set_unique(false);
        CHECK_EQ(bool, unique_sl.is_unique(), false, W("73: set_unique(false)"));
        CHECK_EQ(bool, unique_sl.add(1), true, W("74: Allow duplicate after set_unique(false)"));
        CHECK_EQ(size_t, unique_sl.size(), 2, W("75: Size after allowing duplicate"));
        CHECK_EQ(size_t, unique_sl[1], 1, W("76: Duplicate element at index 1"));

        // Test unique list with custom comparator
        SortedList<size_t, Descending> unique_desc_sl({ 3, 1 }, true, Descending{});
        CHECK_EQ(bool, unique_desc_sl.is_unique(), true, W("77: Unique descending constructor"));
        CHECK_EQ(size_t, unique_desc_sl.size(), 2, W("78: Unique descending list size"));
        CHECK_EQ(size_t, unique_desc_sl[0], 3, W("79: Unique descending first element"));
        CHECK_EQ(size_t, unique_desc_sl[1], 1, W("80: Unique descending second element"));
        CHECK_EQ(bool, unique_desc_sl.add(3), false, W("81: Reject duplicate in unique descending list"));
        CHECK_EQ(size_t, unique_desc_sl.size(), 2, W("82: Size unchanged after rejected duplicate"));
        unique_desc_sl.set_unique(false);
        CHECK_EQ(bool, unique_desc_sl.add(3), true, W("83: Allow duplicate after set_unique(false)"));
        CHECK_EQ(size_t, unique_desc_sl.size(), 3, W("84: Size after allowing duplicate"));
        CHECK_EQ(size_t, unique_desc_sl[1], 3, W("85: Duplicate element at index 1"));

        // Test unique list with string comparator
        SortedList<S, CaseInsensitive> unique_str_sl({ }, true, CaseInsensitive{});
        CHECK_EQ(bool, unique_str_sl.add(W("Apple")), true, W("86: Add to unique string list"));
        CHECK_EQ(bool, unique_str_sl.add(W("apple")), false, W("87: Reject case-insensitive duplicate"));
        CHECK_EQ(size_t, unique_str_sl.size(), 1, W("88: Size after rejected string duplicate"));

        // Test initializer list with empty list
        SortedList<size_t, Descending> empty_desc_sl({}, false, Descending{});
        CHECK_EQ(size_t, empty_desc_sl.size(), 0, W("89: Empty initializer list size"));
        CHECK_EQ(bool, empty_desc_sl.is_unique(), false, W("90: Empty initializer list not unique"));

        // Test initializer list with unique constraint
        SortedList<size_t, Descending> unique_init_sl({ 3, 1, 3, 1 }, true, Descending{});
        CHECK_EQ(size_t, unique_init_sl.size(), 2, W("91: Unique initializer list size"));
        CHECK_EQ(size_t, unique_init_sl[0], 3, W("92: Unique initializer list first element"));
        CHECK_EQ(size_t, unique_init_sl[1], 1, W("93: Unique initializer list second element"));

        // Test case-insensitive comparator order explicitly
        SortedList<S, CaseInsensitive> str_order_sl({ W("Cherry"), W("banana"), W("Apple"), W("date") }, false, CaseInsensitive{});
        CHECK_EQ(S, str_order_sl[0], W("Apple"), W("94: Case-insensitive order first element"));
        CHECK_EQ(S, str_order_sl[1], W("banana"), W("95: Case-insensitive order second element"));
        CHECK_EQ(S, str_order_sl[2], W("Cherry"), W("96: Case-insensitive order third element"));
        CHECK_EQ(S, str_order_sl[3], W("date"), W("97: Case-insensitive order fourth element"));

        // Diagnostic test for str_sl state
        CHECK_EQ(S, str_sl.to_s(), W("[Apple, banana, Cherry, date]"), W("98: Full order string is not corrett."));

        // Additional case-insensitive test with mixed case
        SortedList<S, CaseInsensitive> mixed_case_sl({ W("APPLE"), W("Banana"), W("cherry"), W("Date") }, false, CaseInsensitive{});
        CHECK_EQ(S, mixed_case_sl[0], W("APPLE"), W("99: Mixed case order first element"));
        CHECK_EQ(S, mixed_case_sl[1], W("Banana"), W("100: Mixed case order second element"));
        CHECK_EQ(S, mixed_case_sl[2], W("cherry"), W("101: Mixed case order third element"));
        CHECK_EQ(S, mixed_case_sl[3], W("Date"), W("102: Mixed case order fourth element"));

        // Additional diagnostic for comparator behavior
        CaseInsensitive comp;
        CHECK(comp(W("Cherry"), W("date")), W("103: Error on comparing Cherry and date"));
        CHECK(!comp(W("date"), W("Cherry")), W("104: Error on comparing date and C"));

        TEST_END(SortedList)

    }
}