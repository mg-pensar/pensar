// author : Mauricio Gomes
// license: MIT (https://opensource.org/licenses/MIT)

#ifndef SYSTEM_MACOS_HPP
#define SYSTEM_MACOS_HPP

#include <sys/socket.h>
#include <sys/sysctl.h>
#include <net/if.h>
#include <net/if_dl.h>
#include <net/route.h>
#include <sys/statvfs.h>

#include "string_def.hpp"
#include "mac_address.hpp"
#include "ostype.hpp"

namespace pensar_digital
{
    namespace cpplib
    {
        static S os_name()
        {
            return W("macOS");
        }

        inline static const OSType OS_TYPE = OSType::macOS;
        
        inline static const S LINE_FEED = W("\n");
        inline static constexpr size_t get_max_name_length() noexcept { return 255; /* APFS */ }
        inline static constexpr size_t get_max_path() noexcept
        {
            #define DEFAULT_MACOS_MAX_PATH 1024
            return DEFAULT_MACOS_MAX_PATH;
        }
        inline static constexpr C path_separator() noexcept { return W('/'); }
        
        inline static std::vector<MacAddress> mac_addresses()
        {
            std::vector<MacAddress> result;

            int mib[6] = { CTL_NET, AF_ROUTE, 0, AF_LINK, NET_RT_IFLIST, 0 };
            size_t len = 0;

            // First call to determine buffer size.
            if (sysctl(mib, 6, nullptr, &len, nullptr, 0) < 0)
                return result;

            std::vector<char> buf(len);
            if (sysctl(mib, 6, buf.data(), &len, nullptr, 0) < 0)
                return result;

            char* ptr = buf.data();
            char* end = ptr + len;

            while (ptr < end)
            {
                struct if_msghdr* ifm = reinterpret_cast<struct if_msghdr*>(ptr);
                if (ifm->ifm_type == RTM_IFINFO)
                {
                    struct sockaddr_dl* sdl = reinterpret_cast<struct sockaddr_dl*>(ifm + 1);
                    if (sdl->sdl_alen == 6) // Ethernet MAC address length
                    {
                        unsigned char* mac = reinterpret_cast<unsigned char*>(LLADDR(sdl));
                        // Skip loopback / zero addresses.
                        if (mac[0] || mac[1] || mac[2] || mac[3] || mac[4] || mac[5])
                        {
                            int64_t mac_val =
                                (static_cast<int64_t>(mac[0]) << 40) |
                                (static_cast<int64_t>(mac[1]) << 32) |
                                (static_cast<int64_t>(mac[2]) << 24) |
                                (static_cast<int64_t>(mac[3]) << 16) |
                                (static_cast<int64_t>(mac[4]) << 8)  |
                                 static_cast<int64_t>(mac[5]);
                            result.push_back(MacAddress(mac_val));
                        }
                    }
                }
                ptr += ifm->ifm_msglen;
            }

            return result;
        }

        inline static S cpu_id()
        {
            char buf[256];
            size_t len = sizeof(buf);
            // Try the detailed CPU brand string first (Intel Macs).
            if (sysctlbyname("machdep.cpu.brand_string", buf, &len, nullptr, 0) == 0)
                return S(buf);
            // Fallback to hardware model (works on Apple Silicon and Intel).
            len = sizeof(buf);
            if (sysctlbyname("hw.model", buf, &len, nullptr, 0) == 0)
                return S(buf);
            return W("");
        }
    }       // namespace cpplib
}           // namespace pensar_digital

#endif      // SYSTEM_MACOS_HPP
