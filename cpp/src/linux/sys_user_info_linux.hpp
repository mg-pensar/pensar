// author : Mauricio Gomes
// license: MIT (https://opensource.org/licenses/MIT)

#ifndef SYS_USER_INFO_LINUX_HPP
#define SYS_USER_INFO_LINUX_HPP

#include <cstdlib>
#include <pwd.h>
#include <unistd.h>

#include "path.hpp"

namespace pensar_digital
{
    namespace cpplib
    {
        inline Path get_user_home()
        {
            const char* homeDir = getenv("HOME");
            if (homeDir != nullptr)
            {
                return Path(std::string(homeDir));
            }
            // Fallback: query the password database.
            struct passwd* pw = getpwuid(getuid());
            if (pw != nullptr && pw->pw_dir != nullptr)
            {
                return Path(std::string(pw->pw_dir));
            }
            return Path("");
        }
    } // namespace cpplib
} // namespace pensar_digital

#endif // SYS_USER_INFO_LINUX_HPP
