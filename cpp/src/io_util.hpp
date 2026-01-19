// author : Mauricio Gomes
// license: MIT (https://opensource.org/licenses/MIT)
#ifndef IO_UTIL_HPP
#define IO_UTIL_HPP

#include "multiplatform.hpp"
#include "defines.hpp"
#include "s.hpp"
#include "memory.hpp"
#include "constant.hpp"
#include "concept.hpp"
#include "icu_util.hpp"
#include "log.hpp"
#include "error.hpp"

#include "code_util.hpp"

// Detects and includes platform-specific implementation header.
// Examples:
// #include "windows/io_util_windows.hpp"
// #include "linux/io_util_linux.hpp" 
// #include "android/io_util_android.hpp" or other platforms
#include INCLUDE(io_util)



namespace pensar_digital 
{
    namespace cpplib 
    {

        using LINE_HANDLER = void(*)(const int64_t line_count, const S& line);

        Result<S> get_exe_full_path();


    }  // namespace cpplib
}  // namespace pensar_digital

#endif // IO_UTIL_HPP

