// Guard
#ifndef sys_user_info_windows_hpp
#define sys_user_info_windows_hpp
        
#include <windows.h>
#include <shlobj.h>  // For SHGetFolderPathW
#include <lmcons.h>  // For UNLEN

#include "path.hpp" // Path

namespace
pensar_digital
{
    namespace cpplib
    {
 
        template <bool use_exceptions = true>
        Path get_user_home_windows()
        {
            wchar_t path[MAX_PATH];
            HRESULT result = SHGetFolderPathW(NULL, CSIDL_PROFILE, NULL, 0, path);
            if (result == S_OK)
            {
                return Path(std::wstring(path));
            }
            else
            {
                if (use_exceptions)
                {
                    throw std::runtime_error("get_user_home_windows: Failed to get user home directory.");
                }
                else
                {
                    return Path(L"");
                }
            }
        }
    } // namespace cpplib
} // namespace pensar_digital

#endif // sys_user_info_windows_hpp