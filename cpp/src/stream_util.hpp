// author : Mauricio Gomes
// license: MIT (https://opensource.org/licenses/MIT)

#ifndef STREAM_UTIL_HPP
#define STREAM_UTIL_HPP

#include "s.hpp"

#include <iostream>
#include <sstream>

namespace pensar_digital
{
    namespace cpplib
    {
        inline constexpr OutStream& out() noexcept
        {
            #ifdef WIDE_CHAR
                return std::wcout;
            #else
                return std::cout;
            #endif
        }
        
        inline auto StringStream() noexcept
        {
#ifdef WIDE_CHAR
            return std::wstringstream ();
#else
            return std::stringstream();
#endif
        }


    }       // namespace cpplib
}           // namespace pensar_digital

#endif      // STREAM_UTIL_HPP
