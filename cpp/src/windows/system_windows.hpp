// author : Mauricio Gomes
// license: MIT (https://opensource.org/licenses/MIT)

#ifndef SYSTEM_WINDOWS_HPP
#define SYSTEM_WINDOWS_HPP

#include <winsock2.h>
#include <iphlpapi.h>
#include <intrin.h>
#include <windows.h>

#pragma comment(lib, "IPHLPAPI.lib") // Link with Iphlpapi.lib

// Specialization for Windows
template<>
class System<OS::Windows> : public BaseSystem
        {
        public:
            inline static const S LINE_FEED = W("\r\n");
            inline static size_t get_max_name_length() noexcept { return 255; /* NTFS */ }
            inline static size_t get_max_path() noexcept { return MAX_PATH; /* MAX_PATH */ }

            inline static constexpr C path_separator() noexcept { return W('\\'); }

            inline static bool is_valid_path(const S& path_name)
            {
                return is_name_valid_common(path_name) && path_name.find(W('\0')) == S::npos;
            }

            inline static bool is_valid_file_name(const S& file_name)
            {
                static const S invalidChars = W("<>:\"/\\|?*");
                return is_name_valid_common(file_name) && file_name.find_first_of(invalidChars) == S::npos;
            }

            inline static std::vector<MacAddress> mac_addresses()
            {
                std::vector<MacAddress> mac_addresses;
                PIP_ADAPTER_INFO AdapterInfo;
                DWORD dwBufLen = sizeof(AdapterInfo);
                char* mac_addr = new char[18];

                AdapterInfo = (IP_ADAPTER_INFO*)malloc(sizeof(IP_ADAPTER_INFO));
                if (AdapterInfo == NULL) {
                    printf("Error allocating memory needed to call GetAdaptersinfo\n");
                    return mac_addresses;
                }

                if (GetAdaptersInfo(AdapterInfo, &dwBufLen) == ERROR_BUFFER_OVERFLOW) {
                    AdapterInfo = (IP_ADAPTER_INFO*)malloc(dwBufLen);
                    if (AdapterInfo == NULL) {
                        printf("Error allocating memory needed to call GetAdaptersinfo\n");
                        return mac_addresses;
                    }
                }

                if (GetAdaptersInfo(AdapterInfo, &dwBufLen) == NO_ERROR) {
                    PIP_ADAPTER_INFO pAdapterInfo = AdapterInfo;
                    do {
                        sprintf_s(mac_addr, 18, "%02X:%02X:%02X:%02X:%02X:%02X",
                            pAdapterInfo->Address[0], pAdapterInfo->Address[1],
                            pAdapterInfo->Address[2], pAdapterInfo->Address[3],
                            pAdapterInfo->Address[4], pAdapterInfo->Address[5]);
                        printf("Address: %s, mac: %s\n", pAdapterInfo->IpAddressList.IpAddress.String, mac_addr);

                        int64_t mac;
                        sscanf_s(mac_addr, "%llx", &mac);
                        mac_addresses.push_back(MacAddress(mac));

                        pAdapterInfo = pAdapterInfo->Next;
                    } while (pAdapterInfo);
                }
                free(AdapterInfo);

                return mac_addresses;
            }

            inline static S cpu_id()
            {
                std::array<int, 4> cpuid;
                __cpuid(cpuid.data(), 0);
                S cpu_id = to_string(cpuid[1]) + to_string(cpuid[3]);
                return cpu_id;
            }
        };  // Windows System class.

#endif // SYSTEM_WINDOWS_HPP
