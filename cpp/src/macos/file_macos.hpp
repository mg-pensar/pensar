// author : Mauricio Gomes
// license: MIT (https://opensource.org/licenses/MIT)

#ifndef FILE_MACOS_HPP
#define FILE_MACOS_HPP

#include <cstring>
#include <cerrno>

namespace pensar_digital
{
    namespace cpplib
    {
        // macOS does not provide errno_t; define as int.
        using errno_t = int;

        // Thread-safe strerror wrapper for macOS.
        inline void safe_strerror(char* buffer, size_t buffer_size, int error_code)
        {
            const char* msg = strerror(error_code);
            std::strncpy(buffer, msg ? msg : "", buffer_size);
            if (buffer_size > 0) buffer[buffer_size - 1] = 0;
        }
    }   // namespace cpplib
}       // namespace pensar_digital

#endif  // FILE_MACOS_HPP
