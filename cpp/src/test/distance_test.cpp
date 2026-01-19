#ifndef DISTANCE_TEST_HPP
#define DISTANCE_TEST_HPP

#include <map>

#include "../../../unit_test/src/test.hpp"

#include "../s.hpp"
#include "../distance.hpp"


namespace pensar_digital
{
    namespace test = pensar_digital::unit_test;
    using namespace pensar_digital::unit_test;
    namespace cpplib
    {
        TEST(DistLev, true)
            using Pair = std::pair<S, S>;
            using Map = std::map<Pair, size_t>;
            Map map;
            map[Pair (W(""), W(""))] = 0;
            map[Pair (W(""), W("a"))] = 1;
            map[Pair (W("abc"), W("abd"))] = 1;
            map[Pair (W("abc"), W("ab"))] = 1;
            map[Pair (W("abc"), W("acb"))] = 1;
            map[Pair (W("abc"), W("a"))] = 2;
            map[Pair (W("abc"), W("acd"))] = 2;
            map[Pair (W("abc"), W("agg"))] = 2;
            map[Pair (W("Campeonato Inglês Arsenal x Liverpool"), W("Campeonato Ingles ArsenalxLiverpool"))] = 4;
            map[Pair (W("1234"), W("456"))] = 4;
            map[Pair (W("PLANETA EXPN: SURFE 2009"), W("PLANETA EXPN : AVENTURAS COM RENATA FALZONI"))] = 25;
            map[Pair (W("Copa América Masculino de Basquete: Brasil x Porto Rico"), W("Copa América (r)  Masculino de Basquete -VT- Brasil x Porto Rico"))] = 10;
            map[Pair (W("copaamericamasculinodebasquetebrasilxportorico"), W("copaamericamasculinodebasqueteVTbrasilxportorico"))] = 2;

            size_t i = 0;
            for (const auto& it : map)
            {
                S s = it.first.first;
                S t = it.first.second;
                size_t result = distance (s, t);
				std::stringstream ss;
				ss  << s << " " << t << " " << "i = " << i++;
                CHECK_EQ(size_t, result, it.second, ss.str ());
            }
        TEST_END(DistLev)

        TEST(DistLevIcu, true)
            using Pair = std::pair<S, S>;
            using Map = std::map<Pair, size_t>;
            Map map;
            map[Pair (W(""), W(""))] = 0;
            map[Pair (W(""), W("a"))] = 1;
            map[Pair (W("abc"), W("abd"))] = 1;
            map[Pair (W("abc"), W("ab"))] = 1;
            map[Pair (W("abc"), W("acb"))] = 1;
            map[Pair (W("abc"), W("a"))] = 2;
            map[Pair (W("abc"), W("acd"))] = 2;
            map[Pair (W("abc"), W("agg"))] = 2;
            map[Pair (W("Campeonato Inglês Arsenal x Liverpool"), W("Campeonato Ingles ArsenalxLiverpool"))] = 3;
            map[Pair (W("1234"), W("456"))] = 4;
            map[Pair (W("PLANETA EXPN: SURFE 2009"), W("PLANETA EXPN : AVENTURAS COM RENATA FALZONI"))] = 25;
            map[Pair (W("Copa América Masculino de Basquete: Brasil x Porto Rico"), W("Copa América (r)  Masculino de Basquete -VT- Brasil x Porto Rico"))] = 10;
            map[Pair (W("copaamericamasculinodebasquetebrasilxportorico"), W("copaamericamasculinodebasqueteVTbrasilxportorico"))] = 2;

            size_t i = 0;
            for (const auto& it : map)
            {
                S s = it.first.first;
                S t = it.first.second;
                size_t result = icu_distance(s, t);
                std::stringstream ss;
                ss  << s << " " << t << " " << "i = " << i++;
                CHECK_EQ(size_t, result, it.second, ss.str());
            }
        TEST_END(DistLevIcu)
    }
}
#endif
