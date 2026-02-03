// author : Mauricio Gomes  
// license: MIT (https://opensource.org/licenses/MIT)  
  
#ifndef constantesHPP  
#define constantesHPP  
  
// Defines.  
#define VISUAL_STUDIO  
  
//---------------------------------------------------------------------------  
#define _WINSOCKAPI_  
#include <cstdint>  
#include <vector>  
#include <cstddef> // std::byte  
#include <concepts> // std::signed_integral  
#include <iostream>     // std::cout  
#include <fstream>      // std::ifstream  
#include <string>       // std::string  
#include <sstream>      // std::stringstream  
#include <filesystem>   // std::filesystem  
#include <algorithm>    // std::find  
#include <functional>   // std::function  
#include <memory>       // std::shared_ptr  
#include <map>          // std::map  
#include <set>          // std::set  
#include <list>         // std::list  
#include <queue>        // std::queue  
#include <stack>        // std::stack  
#include <tuple>        // std::tuple  
#include <variant>      // std::variant  
#include <optional>     // std::optional  
#include <any>          // std::any  
#include <chrono>       // std::chrono  
#include <thread>       // std::thread  
#include <mutex>        // std::mutex  
#include <condition_variable> // std::condition_variable  
#include <atomic>       // std::atomic  
#include <future>       // std::future  
#include <type_traits>  // std::is_same  
#include <typeinfo>     // std::type_info  
#include <random>       // std::random_device  
#include <functional>   // std::function  
#include <cctype>       // std::tolower  
#include <span>         // std::span  

#include "string_def.hpp"

  
namespace pensar_digital  
{  
    namespace cpplib  
    {  
        struct Data {};

#if defined(_WIN32) || defined(_WIN64)
#define WINDOWS
#endif

        using Id         = int_fast64_t; // istream >> unsigned long was not compiling.  
        using Hash       = int_fast64_t;  
        using FileSize   = int_fast64_t;  
          
        inline const Id NULL_ID = -1;

          
        template <typename T>  
        concept unsigned_integral = std::is_integral<T>::value && std::is_unsigned<T>::value;  
          
        template <unsigned_integral T>  
        constexpr T null_value() { return 0; }  
          
        template <std::signed_integral T>  
        constexpr T null_value() { return -1; }  
  
        enum Alignment : int_fast16_t { NULL_ALIGNMENT = -1, BIT8 = 8, BIT16 = 16, BIT32 = 32, BIT64 = 64, BIT128 = 128, BIT256 = 256, BIT512 = 512, BIT1024 = 1024 };  
          
        using Byte     = std::byte;  
        using Bytes    = std::vector<Byte>;  
        using BytePtr  = Byte*;  
        using ByteSpan = std::span<Byte>;
        using ConstByteSpan = std::span<const Byte>;  
        inline const bool PAD_RIGHT = true;
        inline const bool PAD_LEFT = false;
        inline const bool TRIM_ELEMENTS = true;
        inline const bool INCLUDE_EMPTY_FIELDS = true;
        inline static const bool ADD_NULL_AT_END = true;  ///< Add null character at the end of the string.
        inline static const bool DO_NOT_ADD_NULL_AT_END = false;  ///< Do not add null character at the end of the string.
        inline static const bool FILL_NULL_BEFORE_COPY = true;  ///< Fill dest memory with null characters before copying the data.
        inline static const bool DO_NOT_FILL_NULL_BEFORE_COPY = false;  ///< Do not fill dest memory with null characters before copying the data.
		inline static const C* CPPLIB_NAMESPACE = W("pensar_digital::cpplib");  ///< Namespace for the library.
		using VersionInt = int16_t;  ///< Type for version integers.
    }   // namespace cpplib  
}       // namespace pensar_digital  
  
namespace pd = pensar_digital::cpplib;  
  
#endif
