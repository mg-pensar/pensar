// author : Mauricio Gomes
// license: MIT (https://opensource.org/licenses/MIT)

#ifndef LINUX_IO_UTIL_HPP
#define LINUX_IO_UTIL_HPP

#include <string>
#include <filesystem>
#include <unistd.h>
#include <limits.h>
#include <cstring>

#include "../string_def.hpp"
#include "../s.hpp"
#include "../code_util.hpp"
#include "../error.hpp"

namespace pensar_digital
{
    namespace cpplib
    {
        namespace fs = std::filesystem;

        // Get the full path of the executable on Linux.
        inline Result<S> get_exe_full_path()
        {
            char buffer[PATH_MAX];
            ssize_t len = readlink("/proc/self/exe", buffer, sizeof(buffer) - 1);
            if (len == -1)
            {
                return Result<S>(W("Error getting executable path"));
            }
            buffer[len] = '\0';

            // Resolve to canonical path
            char resolved[PATH_MAX];
            if (realpath(buffer, resolved) == nullptr)
            {
                return Result<S>(W("Error resolving executable path"));
            }
            return Result<S>(S(resolved));
        }
    }   // namespace cpplib
}       // namespace pensar_digital

#endif  // LINUX_IO_UTIL_HPP
