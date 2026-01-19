#ifndef DISTANCE_HPP_INCLUDED
#define DISTANCE_HPP_INCLUDED

#undef min // avoid conflict with std::min
#include <algorithm> // std::min
#include "s.hpp"


namespace pensar_digital
{
    namespace cpplib
    {
        namespace pd = pensar_digital::cpplib;   

        /// Levenshtein Distance Algorithm
        // adapted from Anders Sewerin Johansen code.

#include <algorithm>
#include <vector>

        inline size_t distance(const S& source, const S& target) {
            const size_t n = source.length();
            const size_t m = target.length();
            if (n == 0) return m;
            if (m == 0) return n;

            // Use three rows to handle transposition
            std::vector<size_t> row1(m + 1);  // i-2
            std::vector<size_t> row2(m + 1);  // i-1
            std::vector<size_t> row3(m + 1);  // i

             for (size_t j = 0; j <= m; j++) {
                row2[j] = j;  // Initialize row2 as first row
            }

            for (size_t i = 1; i <= n; i++) {
                row3[0] = i;

                for (size_t j = 1; j <= m; j++) {
                    const char s_i = source[i - 1];
                    const char t_j = target[j - 1];
                    size_t cost = (s_i == t_j) ? 0 : 1;

                    row3[j] = std::min(std::min(row3[j - 1] + 1, row2[j] + 1), row2[j - 1] + cost);

                    // Transposition: requires i-2 data
                    if (i > 1 && j > 1 && s_i == target[j - 2] && t_j == source[i - 2]) {
                        row3[j] = std::min(row3[j], row1[j - 2] + 1);
                    }
                }

                // Rotate rows: row1 <- row2 <- row3
                row1 = row2;
                row2 = row3;
            }

            return row2[m];  // row2 holds the final result after last iteration
        }

        inline size_t icu_distance(const S& source, const S& target)
        {
            std::vector<UChar32> src = icu_util::to_codepoints(source);
            std::vector<UChar32> tgt = icu_util::to_codepoints(target);

            const size_t n = src.size();
            const size_t m = tgt.size();
            if (n == 0) return m;
            if (m == 0) return n;

            std::vector<size_t> row1(m + 1);  // i-2
            std::vector<size_t> row2(m + 1);  // i-1
            std::vector<size_t> row3(m + 1);  // i

            for (size_t j = 0; j <= m; j++)
            {
                row2[j] = j;
            }

            for (size_t i = 1; i <= n; i++)
            {
                row3[0] = i;

                for (size_t j = 1; j <= m; j++)
                {
                    const UChar32 s_i = src[i - 1];
                    const UChar32 t_j = tgt[j - 1];
                    size_t cost = (s_i == t_j) ? 0 : 1;

                    row3[j] = std::min(std::min(row3[j - 1] + 1, row2[j] + 1), row2[j - 1] + cost);

                    if (i > 1 && j > 1 && s_i == tgt[j - 2] && t_j == src[i - 2])
                    {
                        row3[j] = std::min(row3[j], row1[j - 2] + 1);
                    }
                }

                row1 = row2;
                row2 = row3;
            }

            return row2[m];
        }
 #ifndef LESS_DIFF
#define LESS_DIFF

        struct LessDistance
        {
            S reference;

            LessDistance (const S& s) : reference(s) {}

            bool operator () (const S& left, const S& right)
            {
                //cpplog << "left = " << left << " right = " << right << std::endl;
                return pd::distance (left, reference) < pd::distance (right, reference);
            }
        };
#endif 

        template <class Container = std::vector<S>>
        typename Container::value_type min_distance (const S& s, const Container& c)
        {
            INVALID_ARGUMENT(c.size () == 0, "c.size () = 0");

            typename Container::const_iterator it = c.begin ();
            typename Container::value_type min = *it;
            int min_dist = pd::distance (min, s);
            for (const auto& it : c)
            {
                int dist = pd::distance (it, s);
                if (dist < min_dist)
                {
                    min = it;
                    min_dist = dist;
                }
            }

            return min;
        }

        template <class Container = std::vector<C*>>
        typename Container::value_type min_distance (const C* s, const Container& c)
        {
            return min_distance (S(s), c);
        }

        template <class Container = std::map<S, S>>
        typename Container::value_type min_distance_map_key (const S& s, const Container& c, int max_distance = std::numeric_limits<int>::max())
        {
            INVALID_ARGUMENT(c.size () == 0, "c.size () = 0");

            typename Container::const_iterator it = c.begin ();
            std::pair<typename Container::key_type, typename Container::mapped_type> minimum(it->first, it->second);
            int min_dist = pd::distance (minimum.first, s);
            for (; it != c.end (); ++it)
            {
                int dist = pd::distance (it->first, s);
                if (dist < min_dist)
                {
                    minimum = *it;
                    min_dist = dist;
                }
            }

            std::pair<typename Container::key_type, typename Container::mapped_type> nope;
            return (min_dist <= max_distance) ? minimum : nope;
        }

        template <class Container = std::vector<S>, class OutContainer = std::vector<S>>
        void min_distance (const S& s, const Container& c, OutContainer& out, unsigned max_elements = 0)
        {
            INVALID_ARGUMENT(c.size () == 0, "c.size () = 0");
            LessDistance ld (s);
            using MSet = std::multiset<S, LessDistance>;
            MSet mset (ld);

            for (const auto& it : mset)
            {
                mset.insert (it);
            }
            if (max_elements == 0)
                std::copy (mset.begin (), mset.end (), std::back_inserter(out));
            else
            {
                typename MSet::iterator fim = mset.begin ();
                while (max_elements--)
                    ++fim;
                std::copy (mset.begin (), fim, std::back_inserter(out));
            }
        }


        template <class Container = std::vector<C*>, class OutContainer = std::vector<C*>>
        void min_distance (const C* s, const Container& c, OutContainer& out, unsigned max_elements = 0)
        {
            min_distance (S(s), c, out, max_elements);
        }
 

    }   // namespace cpplib
}       // namespace pensar_digital

#endif // DISTANCE_HPP_INCLUDED
