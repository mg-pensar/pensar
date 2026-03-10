// author : Mauricio Gomes
// license: MIT (https://opensource.org/licenses/MIT)

#ifndef FILE_WINDOWS_HPP
#define FILE_WINDOWS_HPP

#include <cstring>
#include <cerrno>

namespace pensar_digital
{
    namespace cpplib
    {
        // errno_t is natively available on MSVC.

        // Thread-safe strerror wrapper for Windows using strerror_s.
        inline void safe_strerror(char* buffer, size_t buffer_size, errno_t error_code)
        {
            strerror_s(buffer, buffer_size, error_code);
        }
    }   // namespace cpplib
}       // namespace pensar_digital

#endif  // FILE_WINDOWS_HPP
