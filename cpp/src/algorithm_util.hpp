// $Id
#ifndef ALGORITHM_UTIL_H_INCLUDED
#define ALGORITHM_UTIL_H_INCLUDED

#include<map>
#include<string>
#include<algorithm>
#include<concepts>
#include<iterator>

#include "concept.hpp"

namespace pensar_digital
{
    namespace cpplib
    {
        template<IsContainer C, typename Predicate>
        C& erase_if(C& c, Predicate p)
        {
            c.erase (std::remove_if (c.begin (), c.end (), p), c.end ());
            return c;
        }

        template<typename Map, typename Predicate>
        requires requires(Map& m)
        {
            typename Map::iterator;
            typename Map::key_type;
            typename Map::mapped_type;
            { m.begin() } -> std::same_as<typename Map::iterator>;
            { m.end() } -> std::same_as<typename Map::iterator>;
            { m.erase(m.begin()) } -> std::same_as<typename Map::iterator>;
        } && std::same_as<typename Map::value_type, std::pair<const typename Map::key_type, typename Map::mapped_type>>
        void erase_if(Map& map, Predicate p)
        {
            for (auto it = map.begin(); it != map.end(); )
            {
                if (p(*it))
                    it = map.erase(it);
                else
                    ++it;
            }
        }

        template<
            std::input_iterator In,
            std::sentinel_for<In> Sent,
            std::weakly_incrementable Out,
            typename Pred
        >
        requires std::indirect_unary_predicate<Pred, In> &&
                 std::indirectly_writable<Out, std::iter_reference_t<In>>
        Out copy_if(In first, Sent last, Out res, Pred pr)
        {
            while (first != last)
            {
              if (pr(*first))
                *res++ = *first;
              ++first;
            }

            return res;
        }
    }   // namespace cpplib
}       // namespace pensar_digital
#endif // ALGORITHM_UTIL_H_INCLUDED
