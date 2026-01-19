// author : Mauricio Gomes
// license: MIT (https://opensource.org/licenses/MIT)

#ifndef SYSTEM_HPP
#define SYSTEM_HPP

#include "multiplatform.hpp"
#include "defines.hpp"
#include "s.hpp"
#include "mac_address.hpp"

#ifdef __APPLE__
    #include <TargetConditionals.h>
#endif

#include <iostream>
#include <type_traits>
#include <string>
#include <bit>  // std::endian
#include <cstdint>
#include <array>
#include <vector>

namespace pensar_digital
{
	namespace cpplib
	{
        static S os_name()
        {
            #if defined(_WIN32) || defined(_WIN64)
                return W("Windows");
            #elif defined(__APPLE__) && defined(TARGET_OS_IOS)
                return W("iOS");
            #elif defined(__ANDROID__)
                return W("Android");
            #elif defined(__linux__)
                return W("Linux");
            #elif defined(__APPLE__)
                return W("macOS");
            #else
                return W("Other");
            #endif
        }

        enum class OS
        {
            Windows,
            Linux,
            MacOS,
            IOS,
            Android,
            Other
        };

        inline static constexpr OS os()
        {
            #if defined(_WIN32) || defined(_WIN64)
                return OS::Windows;
            #elif defined(__ANDROID__)
                return OS::Android;
            #elif defined(__APPLE__)
                #if TARGET_OS_IOS
                    return OS::IOS;
                #else
                    return OS::MacOS;
                #endif
            #elif defined(__linux__)
                return OS::Linux;
            #else
                return OS::Other;
            #endif
        }

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

        template<OS OSTtype>
        class System : public BaseSystem
        {
            // Throws an exception if the operating system is not supported.
            static_assert(OSTtype == OS::Windows || OSTtype == OS::Linux || OSTtype == OS::MacOS || OSTtype == OS::IOS || OSTtype == OS::Android, "Unsupported operating system.");            
        };

        // Include platform-specific implementation
        #include INCLUDE(system)

        using Sys = System<os()>;
        inline const static S LF = Sys::LINE_FEED;
        inline static bool is_valid_path(const S& path_name) { return Sys::is_valid_path(path_name); }
        inline static bool is_valid_file_name(const S& file_name) { return Sys::is_valid_file_name(file_name); }
        inline static std::vector<MacAddress> mac_addresses() { return Sys::mac_addresses(); }
        inline static S cpu_id() { return Sys::cpu_id(); }

        // Gets file name from a path string.
        inline static S file_name(const S& path_name)
        {
            size_t pos = path_name.find_last_of(Sys::path_separator());
            if (pos == S::npos)
                return path_name;
            return path_name.substr(pos + 1);
        }
    } // namespace cpplib
} // namespace pensar_digital
#endif	// SYSTEM_HPP
