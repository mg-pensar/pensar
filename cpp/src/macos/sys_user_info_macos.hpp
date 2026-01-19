// Guard
#ifndef sys_user_info_macos_hpp
#define sys_user_info_macos_hpp

#include "path.hpp"

namespace pensar_digital
{
    namespace cpplib
    {
        inline Path get_user_home ()
        {
            // macOS implementation
            const char* homeDir = getenv("HOME");
            if (homeDir != nullptr)
            {
                return Path(std::string(homeDir));
            }
            return Path("");
        }
    } // namespace cpplib
} // namespace pensar_digital

#endif // sys_user_info_macos_hpp