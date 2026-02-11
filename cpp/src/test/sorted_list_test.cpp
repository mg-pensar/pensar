// author: Mauricio Gomes
// license: MIT (https://opensource.org/licenses/MIT)

#include <catch2/catch_test_macros.hpp>

#include "../sorted_list.hpp"
#include <cwctype>
#include <clocale>

namespace pensar_digital::cpplib
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

    TEST_CASE("SortedList", "[sorted_list]")
    {
        // Set locale for consistent case conversion
        std::setlocale(LC_ALL, "C");

        // Basic functionality with default comparator (ascending)
        SortedList<size_t> sl = {};
        INFO(W("0: Default constructor size")); CHECK(sl.size() == 0);
        INFO(W("1: Default constructor not unique")); CHECK(sl.is_unique() == false);

        sl.add(2);
        INFO(W("2: Size after adding one element")); CHECK(sl.size() == 1);
        INFO(W("3: Element at index 0")); CHECK(sl[0] == 2);

        sl.add(0);
        INFO(W("4: Size after adding second element")); CHECK(sl.size() == 2);
        INFO(W("5: Element at index 0 after sorting")); CHECK(sl[0] == 0);
        INFO(W("6: Element at index 1 after sorting")); CHECK(sl[1] == 2);

        sl.add(1);
        INFO(W("7: Size after adding third element")); CHECK(sl.size() == 3);
        INFO(W("8: Element at index 1 after sorting")); CHECK(sl[1] == 1);

        // Test custom comparator (descending)
        SortedList<size_t, Descending> desc_sl({ 3, 1, 4, 1, 5 }, false, Descending{});
        INFO(W("9: Descending list size")); CHECK(desc_sl.size() == 5);
        INFO(W("10: Descending list first element")); CHECK(desc_sl[0] == 5);
        INFO(W("11: Descending list second element")); CHECK(desc_sl[1] == 4);
        INFO(W("12: Descending list third element")); CHECK(desc_sl[2] == 3);
        INFO(W("13: Descending list fourth element")); CHECK(desc_sl[3] == 1);
        INFO(W("14: Descending list fifth element")); CHECK(desc_sl[4] == 1);

        // Test add with custom comparator
        INFO(W("15: Add 2 to descending list")); CHECK(desc_sl.add(2) == true);
        INFO(W("16: Size after adding to descending list")); CHECK(desc_sl.size() == 6);
        INFO(W("17: Element at index 3 after adding 2")); CHECK(desc_sl[3] == 2);

        // Test find
        auto it = sl.find(1);
        INFO(W("18: Find existing element")); CHECK((it != sl.end() ? *it : (size_t)-1) == 1);
        it = sl.find(99);
        INFO(W("19: Find non-existing element")); CHECK((it == sl.end()) == true);

        // Test contains
        INFO(W("20: Contains existing element")); CHECK(sl.contains(2) == true);
        INFO(W("21: Contains non-existing element")); CHECK(sl.contains(99) == false);

        // Test remove
        INFO(W("22: Remove existing element")); CHECK(sl.remove(1) == true);
        INFO(W("23: Size after remove")); CHECK(sl.size() == 2);
        INFO(W("24: Element removed")); CHECK(sl.contains(1) == false);
        INFO(W("25: Remove non-existing element")); CHECK(sl.remove(99) == false);

        // Test remove_at
        sl.remove_at(0);
        INFO(W("26: Size after remove_at")); CHECK(sl.size() == 1);
        INFO(W("27: Element after remove_at")); CHECK(sl[0] == 2);

        // Test iterator
        int expected[] = { 2 };
        int i = 0;
        for (const auto& item : sl) {
            INFO(W("28: Iterator element at index ") + std::to_string(i));
            CHECK(item == expected[i]);
            ++i;
        }
        INFO(W("29: Iterator count")); CHECK(i == 1);

        // Test range
        i = 0;
        for (const auto& item : sl.as_range()) {
            INFO(W("30: Range element at index ") + std::to_string(i));
            CHECK(item == expected[i]);
            ++i;
        }
        INFO(W("31: Range count")); CHECK(i == 1);

        // Test clear
        sl.clear();
        INFO(W("32: Size after clear")); CHECK(sl.size() == 0);
        INFO(W("33: Empty after clear")); CHECK(sl.empty() == true);

        // Test error handling for operator[]
        bool exception_thrown = false;
        try {
            sl[0];
        }
        catch (const std::out_of_range&) {
            exception_thrown = true;
        }
        INFO(W("34: operator[] throws on empty list")); CHECK(exception_thrown == true);

        // Test error handling for at
        exception_thrown = false;
        try {
            sl.at(0);
        }
        catch (const std::out_of_range&) {
            exception_thrown = true;
        }
        INFO(W("35: at throws on empty list")); CHECK(exception_thrown == true);

        // Test reverse iterator
        desc_sl.clear();
        desc_sl.add(3);
        desc_sl.add(2);
        desc_sl.add(1);
        int rev_expected[] = { 1, 2, 3 };
        i = 0;
        for (auto rit = desc_sl.rbegin(); rit != desc_sl.rend(); ++rit) {
            INFO(W("36: Reverse iterator element at index ") + std::to_string(i));
            CHECK(*rit == rev_expected[i]);
            ++i;
        }
        INFO(W("37: Reverse iterator count")); CHECK(i == 3);

        // Test comparator access
        INFO(W("38: Comparator check (5 > 3)")); CHECK(desc_sl.comparator()(5, 3) == true);

        // Test operator[] with custom comparator
        INFO(W("39: Descending list operator[] at index 0")); CHECK(desc_sl[0] == 3);
        INFO(W("40: Descending list operator[] at index 1")); CHECK(desc_sl[1] == 2);
        INFO(W("41: Descending list operator[] at index 2")); CHECK(desc_sl[2] == 1);

        // Test sorted order after multiple adds
        SortedList<size_t> asc_sl2 = {};
        asc_sl2.add(5);
        asc_sl2.add(2);
        asc_sl2.add(8);
        asc_sl2.add(1);
        size_t asc_expected[] = { 1, 2, 5, 8 };
        i = 0;
        for (const auto& item : asc_sl2) {
            INFO(W("42: Ascending order after multiple adds at index ") + std::to_string(i));
            CHECK(item == asc_expected[i]);
            ++i;
        }
        INFO(W("43: Ascending order count")); CHECK(i == 4);

        // Test case-insensitive string comparator
        SortedList<S, CaseInsensitive> str_sl({ W("Apple"), W("banana"), W("date") }, false, CaseInsensitive{});
        INFO(W("44: String list size")); CHECK(str_sl.size() == 3);
        INFO(W("45: String list first element (case-insensitive)")); CHECK(str_sl[0] == W("Apple"));
        INFO(W("46: String list second element (case-insensitive)")); CHECK(str_sl[1] == W("banana"));
        INFO(W("47: String list third element (case-insensitive)")); CHECK(str_sl[2] == W("date"));
        INFO(W("48: Add 'Cherry' to string list")); CHECK(str_sl.add(W("Cherry")) == true);
        INFO(W("49: String list after adding 'date'")); CHECK(str_sl[3] == W("date"));
        INFO(W("50: to_s after adding 'Cherry'")); CHECK(str_sl.to_s() == W("[Apple, banana, Cherry, date]"));

        // Test to_s and conversion to S
        INFO(W("51: to_s on empty list")); CHECK(sl.to_s() == W("[]"));
        INFO(W("52: Conversion to S on empty list")); CHECK(S(sl) == W("[]"));
        INFO(W("53: to_s on ascending list")); CHECK(asc_sl2.to_s() == W("[1, 2, 5, 8]"));
        INFO(W("54: Conversion to S on ascending list")); CHECK(S(asc_sl2) == W("[1, 2, 5, 8]"));
        INFO(W("55: to_s on descending list")); CHECK(desc_sl.to_s() == W("[3, 2, 1]"));
        INFO(W("56: Conversion to S on descending list")); CHECK(S(desc_sl) == W("[3, 2, 1]"));

        // Test copy constructor
        SortedList<size_t, Descending> desc_sl_copy = desc_sl;
        INFO(W("57: Copy constructor size")); CHECK(desc_sl_copy.size() == 3);
        INFO(W("58: Copy constructor first element")); CHECK(desc_sl_copy[0] == 3);
        INFO(W("59: Copy constructor comparator")); CHECK(desc_sl_copy.comparator()(5, 3) == true);

        // Test move constructor
        SortedList<size_t, Descending> desc_sl_move = std::move(desc_sl_copy);
        INFO(W("60: Move constructor size")); CHECK(desc_sl_move.size() == 3);
        INFO(W("61: Move constructor first element")); CHECK(desc_sl_move[0] == 3);
        INFO(W("62: Moved-from list size")); CHECK(desc_sl_copy.size() == 0);

        // Test empty iterator
        i = 0;
        for (const auto& item : sl) {
            i++;
        }
        INFO(W("63: Empty iterator count")); CHECK(i == 0);

        // Test adding duplicates with custom comparator
        INFO(W("64: Add duplicate to non-unique descending list")); CHECK(desc_sl.add(2) == true);
        INFO(W("65: Size after adding duplicate")); CHECK(desc_sl.size() == 4);
        INFO(W("66: First duplicate at index 1")); CHECK(desc_sl[1] == 2);
        INFO(W("67: Second duplicate at index 2")); CHECK(desc_sl[2] == 2);

        // Test unique list with default comparator
        SortedList<size_t> unique_sl({}, true);
        INFO(W("68: Unique constructor")); CHECK(unique_sl.is_unique() == true);
        INFO(W("69: Add to unique list")); CHECK(unique_sl.add(1) == true);
        INFO(W("70: Size after first add to unique list")); CHECK(unique_sl.size() == 1);
        INFO(W("71: Reject duplicate in unique list")); CHECK(unique_sl.add(1) == false);
        INFO(W("72: Size unchanged after rejected duplicate")); CHECK(unique_sl.size() == 1);
        unique_sl.set_unique(false);
        INFO(W("73: set_unique(false)")); CHECK(unique_sl.is_unique() == false);
        INFO(W("74: Allow duplicate after set_unique(false)")); CHECK(unique_sl.add(1) == true);
        INFO(W("75: Size after allowing duplicate")); CHECK(unique_sl.size() == 2);
        INFO(W("76: Duplicate element at index 1")); CHECK(unique_sl[1] == 1);

        // Test unique list with custom comparator
        SortedList<size_t, Descending> unique_desc_sl({ 3, 1 }, true, Descending{});
        INFO(W("77: Unique descending constructor")); CHECK(unique_desc_sl.is_unique() == true);
        INFO(W("78: Unique descending list size")); CHECK(unique_desc_sl.size() == 2);
        INFO(W("79: Unique descending first element")); CHECK(unique_desc_sl[0] == 3);
        INFO(W("80: Unique descending second element")); CHECK(unique_desc_sl[1] == 1);
        INFO(W("81: Reject duplicate in unique descending list")); CHECK(unique_desc_sl.add(3) == false);
        INFO(W("82: Size unchanged after rejected duplicate")); CHECK(unique_desc_sl.size() == 2);
        unique_desc_sl.set_unique(false);
        INFO(W("83: Allow duplicate after set_unique(false)")); CHECK(unique_desc_sl.add(3) == true);
        INFO(W("84: Size after allowing duplicate")); CHECK(unique_desc_sl.size() == 3);
        INFO(W("85: Duplicate element at index 1")); CHECK(unique_desc_sl[1] == 3);

        // Test unique list with string comparator
        SortedList<S, CaseInsensitive> unique_str_sl({}, true, CaseInsensitive{});
        INFO(W("86: Add to unique string list")); CHECK(unique_str_sl.add(W("Apple")) == true);
        INFO(W("87: Reject case-insensitive duplicate")); CHECK(unique_str_sl.add(W("apple")) == false);
        INFO(W("88: Size after rejected string duplicate")); CHECK(unique_str_sl.size() == 1);

        // Test initializer list with empty list
        SortedList<size_t, Descending> empty_desc_sl({}, false, Descending{});
        INFO(W("89: Empty initializer list size")); CHECK(empty_desc_sl.size() == 0);
        INFO(W("90: Empty initializer list not unique")); CHECK(empty_desc_sl.is_unique() == false);

        // Test initializer list with unique constraint
        SortedList<size_t, Descending> unique_init_sl({ 3, 1, 3, 1 }, true, Descending{});
        INFO(W("91: Unique initializer list size")); CHECK(unique_init_sl.size() == 2);
        INFO(W("92: Unique initializer list first element")); CHECK(unique_init_sl[0] == 3);
        INFO(W("93: Unique initializer list second element")); CHECK(unique_init_sl[1] == 1);

        // Test case-insensitive comparator order explicitly
        SortedList<S, CaseInsensitive> str_order_sl({ W("Cherry"), W("banana"), W("Apple"), W("date") }, false, CaseInsensitive{});
        INFO(W("94: Case-insensitive order first element")); CHECK(str_order_sl[0] == W("Apple"));
        INFO(W("95: Case-insensitive order second element")); CHECK(str_order_sl[1] == W("banana"));
        INFO(W("96: Case-insensitive order third element")); CHECK(str_order_sl[2] == W("Cherry"));
        INFO(W("97: Case-insensitive order fourth element")); CHECK(str_order_sl[3] == W("date"));

        // Diagnostic test for str_sl state
        INFO(W("98: Full order string is not corrett.")); CHECK(str_sl.to_s() == W("[Apple, banana, Cherry, date]"));

        // Additional case-insensitive test with mixed case
        SortedList<S, CaseInsensitive> mixed_case_sl({ W("APPLE"), W("Banana"), W("cherry"), W("Date") }, false, CaseInsensitive{});
        INFO(W("99: Mixed case order first element")); CHECK(mixed_case_sl[0] == W("APPLE"));
        INFO(W("100: Mixed case order second element")); CHECK(mixed_case_sl[1] == W("Banana"));
        INFO(W("101: Mixed case order third element")); CHECK(mixed_case_sl[2] == W("cherry"));
        INFO(W("102: Mixed case order fourth element")); CHECK(mixed_case_sl[3] == W("Date"));

        // Additional diagnostic for comparator behavior
        CaseInsensitive comp;
        INFO(W("103: Error on comparing Cherry and date")); CHECK(comp(W("Cherry"), W("date")));
        INFO(W("104: Error on comparing date and C")); CHECK(!comp(W("date"), W("Cherry")));
    }
}
