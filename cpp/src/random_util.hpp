#ifndef RANDOM_UTIL_HPP_INCLUDED
#define RANDOM_UTIL_HPP_INCLUDED

#include <chrono>
#include <random>
#include <cstdlib>
#include <limits>

namespace pensar_digital
{
  namespace cpplib
  {
      using DefaultRandomGeneratorType = unsigned long long int;
      
      inline DefaultRandomGeneratorType seed()
      {
          return std::chrono::system_clock::now().time_since_epoch().count();
      }

      template <typename T = DefaultRandomGeneratorType>
      class CustomRandomGenerator
      {
        public:
          using result_type = T;
          
          #undef min  // Undefine the macro min defined in windows.h to avoid conflict with the member function min.

          CustomRandomGenerator(result_type seed) : m_engine(seed), m_dist(std::numeric_limits<result_type>::min(), std::numeric_limits<result_type>::max()) {}

          static constexpr result_type min() { return std::numeric_limits<result_type>::min(); }
          static constexpr result_type max() { return std::numeric_limits<result_type>::max(); }

          result_type operator()() {
              return m_dist(m_engine);
          }

      private:
          std::mt19937_64 m_engine;
          std::uniform_int_distribution<result_type> m_dist;
      };

      using RandGen = CustomRandomGenerator<>;
    
    template <typename T = DefaultRandomGeneratorType, class Distribution = std::uniform_int_distribution<T>, class RandomGenerator = RandGen>
    class Random 
    {
        public:

        Random(const T min_value, const T max_value) : 
            mseed(seed()),
            mmin(min_value),
            mmax(max_value),
            mdistribution(min_value, max_value),
            mgenerator(mseed)
        {
        }

        T operator()() 
        {
            return mdistribution(mgenerator);
        }

    private:
        DefaultRandomGeneratorType mseed;
        T mmin;
        T mmax;
        Distribution mdistribution;
        RandomGenerator mgenerator;
    };

    class CRandom
    {
      public:
        CRandom (int min_value, int max_value): _min(min_value), _max(max_value) { srand ((unsigned int) seed()); }

        inline int get () const { return rand () % (_max - _min + 1) + _min;}
        inline int get_min () const { return _min; }
        inline int get_max () const { return _max; }

        inline void set_min (const int min_value) { _min = min_value; }
        inline void set_max (const int max_value) { _max = max_value; }
      private:
        int _min;
        int _max;
    };

    inline RandGen get_generator()
    {
        static RandGen random_generator(seed())
            ;

        return random_generator;
    }

    inline RandGen random_generator = get_generator ();
  }
}

#endif // RANDOM_UTIL_HPP_INCLUDED
