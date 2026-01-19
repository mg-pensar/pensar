// author : Mauricio Gomes
// license: MIT (https://opensource.org/licenses/MIT)#include "../algorithm_util.hpp"

#include <unordered_map>
#include <cctype>	// std::isdigit
#include <functional>	// std::bind2nd


#include "../../../unit_test/src/test.hpp"

#include "../algorithm_util.hpp"

#include "../s.hpp"

namespace pensar_digital
{
	namespace test = pensar_digital::unit_test;
	using namespace pensar_digital::unit_test;
	namespace cpplib
	{
		TEST(algorithm_util, true)
			using Map = std::unordered_map<int, S>;
			Map map;
			map[0] = "a";
			map[1] = "1";
			map[2] = "c";
			
			// Erase entries where the value is a digit.
			pd::erase_if(map, [](const auto& pair) { return std::isdigit(pair.second[0]); });

			CHECK_EQ(S, map[0], W("a"), W("0"));
			CHECK_EQ(S, W("c"), map[2], W("1"));
			CHECK_EQ(int, 2u, map.size(), W("2"));
			std::vector<int> v(3);
			v[0] = 0;
			v[1] = 1;
			v[2] = 2;

			// Erase odd entries.
			pd::erase_if(v, [](const auto& value) { return (value % 2) != 0; });

			CHECK_EQ(int, 0, v[0], W("3"));
			CHECK_EQ(int, 2, v[1], W("4"));
			CHECK_EQ(int, 2u, v.size(), W("5"));
		TEST_END(algorithm_util);
	}
}
