// author : Mauricio Gomes
// license: MIT (https://opensource.org/licenses/MIT)

#include <catch2/catch_test_macros.hpp>

#include "../algorithm_util.hpp"
#include "../s.hpp"

#include <unordered_map>
#include <cctype>
#include <functional>

namespace pensar_digital::cpplib
{
    TEST_CASE("algorithm_util", "[algorithm]")
    {
        using Map = std::unordered_map<int, S>;
        Map map;
        map[0] = "a";
        map[1] = "1";
        map[2] = "c";

        // Erase entries where the value is a digit.
        pd::erase_if(map, [](const auto& pair) { return std::isdigit(pair.second[0]); });

        INFO(W("0")); CHECK(map[0] == W("a"));
        INFO(W("1")); CHECK(W("c") == map[2]);
        INFO(W("2")); CHECK(2u == map.size());

        std::vector<int> v(3);
        v[0] = 0;
        v[1] = 1;
        v[2] = 2;

        // Erase odd entries.
        pd::erase_if(v, [](const auto& value) { return (value % 2) != 0; });

        INFO(W("3")); CHECK(0 == v[0]);
        INFO(W("4")); CHECK(2 == v[1]);
        INFO(W("5")); CHECK(2u == v.size());
    }
}
