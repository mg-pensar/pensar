// author : Mauricio Gomes
// license: MIT (https://opensource.org/licenses/MIT)

#ifndef SYSTEM_LINUX_HPP
#define SYSTEM_LINUX_HPP

#include <unistd.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <linux/if.h>
#include <linux/sockios.h>
#include <sys/statvfs.h>
#include <cstring>
#include <cstdio>

namespace pensar_digital
{
    namespace cpplib
    {
        // Specialization for Linux
        template<>
        class System<OS::Linux> : public BaseSystem
        {
        public:
            inline static const S LINE_FEED = W("\n");

            inline static size_t get_max_name_length() noexcept { return 255; /* ext4 */ }
            inline static size_t get_max_path() noexcept
            {
                #define DEFAULT_LINUX_MAX_PATH 4096

                #ifdef PATH_MAX 
                    return PATH_MAX;
                #else
                    return DEFAULT_LINUX_MAX_PATH;
                #endif
            }

            inline static constexpr C path_separator() { return W('/'); }
            
            inline static bool is_valid_file_name(const S& file_name)
            {
                return is_name_valid_common(file_name) && file_name.find(path_separator()) == S::npos && file_name.find('\0') == S::npos;
            }

            inline static bool is_valid_path(const S& path_name)
            {
                return is_name_valid_common(path_name) && path_name.find('\0') == S::npos;
            }

            inline static std::vector<MacAddress> mac_addresses()
            {
                std::vector<MacAddress> mac_addresses;
                struct ifreq ifr;
                struct ifconf ifc;
                char buf[1024];

                int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
                if (sock == -1) { /* handle error*/ }

                ifc.ifc_len = sizeof(buf);
                ifc.ifc_buf = buf;
                if (ioctl(sock, SIOCGIFCONF, &ifc) == -1) { /* handle error */ }

                struct ifreq* it = ifc.ifc_req;
                const struct ifreq* const end = it + (ifc.ifc_len / sizeof(struct ifreq));

                for (; it != end; ++it) {
                    strcpy(ifr.ifr_name, it->ifr_name);
                    if (ioctl(sock, SIOCGIFFLAGS, &ifr) == 0) {
                        if (!(ifr.ifr_flags & IFF_LOOPBACK)) { // don't count loopback
                            if (ioctl(sock, SIOCGIFHWADDR, &ifr) == 0) {
                                unsigned char mac_address[6];
                                memcpy(mac_address, ifr.ifr_hwaddr.sa_data, 6);

                                int64_t mac;
                                sscanf((char*)mac_address, "%llx", &mac);
                                mac_addresses.push_back(MacAddress(mac));
                            }
                        }
                    }
                    else { /* handle error */ }
                }
                
                return mac_addresses;
            }

            static inline S cpu_id()
            {
                S cpu_id;
                InFStream cpuinfo(W("/proc/cpuinfo"));
                if (cpuinfo) 
                {
                    S line;
                    while (std::getline(cpuinfo, line)) {
                        if (line.substr(0, 9) == W("processor"))
                        {
                            cpu_id = line;
                            break;
                        }
                    }
                }
                return cpu_id;
            }
        };

#endif // SYSTEM_LINUX_HPP
