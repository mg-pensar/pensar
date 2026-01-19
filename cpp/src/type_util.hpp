// author : Mauricio Gomes
// license: MIT (https://opensource.org/licenses/MIT)

#ifndef TYPE_UTIL_HPP_INCLUDED
#define TYPE_UTIL_HPP_INCLUDED

#include <typeinfo>
    
#include "constant.hpp"
#include "s.hpp"

namespace pensar_digital
{
    namespace cpplib
    {
        template <class T>
        S class_name() noexcept
        {
            std::string s = typeid(T).name();
            s.erase(0, sizeof(W("class ")) - 1); // remove "class " prefix;
            #ifdef WIDE_CHAR
                return to_wstring (s);
            #else
                return s;
            #endif
        }
        
    }   // namespace cpplib
}   // namespace pensar_digital

#endif // TYPE_UTIL_HPP_INCLUDED#pragma once
