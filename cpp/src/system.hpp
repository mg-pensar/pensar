// author : Mauricio Gomes
// license: MIT (https://opensource.org/licenses/MIT)

#ifndef SYSTEM_HPP
#define SYSTEM_HPP


// ---------------------------------------------------------------------------
// Windows SDK headers MUST be included at global scope, before any namespace.
// system_windows.hpp is later #include'd inside namespace pensar_digital::cpplib
// via the INCLUDE(system) macro.  Without these pre-includes the SDK types
// (DWORD, HWND, HINSTANCE …) would end up scoped inside the namespace, and
// every subsequent SDK header (<shlobj.h>, <commctrl.h>, …) would fail to
// find them.  The include-guards inside the SDK headers make these harmless
// no-ops when system_windows.hpp re-includes them later.
// ---------------------------------------------------------------------------
#if defined(_WIN32) || defined(_WIN64)
#include <winsock2.h>
#include <iphlpapi.h>
#include <windows.h>
#endif

#include <iostream>
#include <type_traits>
#include <string>
#include <bit>  // std::endian
#include <cstdint>
#include <array>
#include <vector>

// Include platform-specific implementation
#include "multiplatform.hpp"
#include INCLUDE(system)
#include "string_def.hpp"
#include "mac_address.hpp"
#include "ostype.hpp"

#ifdef __APPLE__
    #include <TargetConditionals.h>
#endif

namespace pensar_digital
{
	namespace cpplib
	{        
        class BaseSystem
        {
            public:
                // Common constraints
                inline static constexpr size_t DEFAULT_MAX_NAME_LENGTH = 255;
                inline static constexpr size_t DEFAULT_MAX_PATH = 260;

                inline static S endianess_name() noexcept { return (std::endian::native == std::endian::little) ? W("Little Endian") : W("Big Endian"); }

                static std::endian endianess() noexcept
                {
                    return (std::endian::native == std::endian::little) ? std::endian::little : std::endian::big;
                }

                // Helper function to check the first character and max size
                inline static bool is_name_valid_common(const S& name) {
                    return !name.empty() && name[0] != '.' && name.size() <= DEFAULT_MAX_NAME_LENGTH;
                }
        };
       
        class System : public BaseSystem
        {
            public:

                inline static const S LF = LINE_FEED;
                inline static const size_t MAX_NAME_LENGTH = get_max_name_length();
                inline static const size_t MAX_PATH        = get_max_path();
                inline static const      C PATH_SEPARATOR  = path_separator();
                inline static const S OS_NAME = os_name();
                inline static const OSType OS = OS_TYPE;
                inline static bool is_valid_file_name(const S& file_name)
                {
                    return is_name_valid_common(file_name) && file_name.find(path_separator()) == S::npos && file_name.find('\0') == S::npos;
                }

                inline static bool is_valid_path(const S& path_name)
                {
                    return is_name_valid_common(path_name) && path_name.find(W('\0')) == S::npos;
                }

                inline static std::vector<MacAddress> mac_addresses();

                inline static S cpu_id();
        };  // class System

        using Sys = System;
        inline const static S LF = Sys::LF;

     } // namespace cpplib
} // namespace pensar_digital
#endif	// SYSTEM_HPP
