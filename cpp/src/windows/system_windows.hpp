// author : Mauricio Gomes
// license: MIT (https://opensource.org/licenses/MIT)

#ifndef SYSTEM_WINDOWS_HPP
#define SYSTEM_WINDOWS_HPP

#include <winsock2.h>
#include <iphlpapi.h>
#include <intrin.h>
#include <windows.h>

#include <string>
#include <vector>
#include <array>
#include <cstdint>

#include "string_def.hpp"
#include "mac_address.hpp"
#include "ostype.hpp"

#pragma comment(lib, "IPHLPAPI.lib") // Link with Iphlpapi.lib

namespace pensar_digital
{
    namespace cpplib
    {
        inline static S os_name()
        {
            return W("Windows");
        }

        inline static const OSType OS_TYPE = OSType::Windows;

        inline static const S LINE_FEED = W("\n");
        inline static constexpr size_t get_max_name_length() noexcept { return 255; /* NTFS */ }
        inline static constexpr size_t get_max_path() noexcept
        {
            #define DEFAULT_WINDOWS_MAX_PATH 1024
            return DEFAULT_WINDOWS_MAX_PATH;
        }
        inline static constexpr C path_separator() noexcept { return W('\\'); }

        inline static std::vector<MacAddress> mac_addresses()
        {
            std::vector<MacAddress> result;
            PIP_ADAPTER_INFO AdapterInfo;
            DWORD dwBufLen = sizeof(IP_ADAPTER_INFO);
            char mac_addr[18];

            AdapterInfo = (IP_ADAPTER_INFO*)malloc(sizeof(IP_ADAPTER_INFO));
            if (AdapterInfo == NULL) {
                return result;
            }

            if (GetAdaptersInfo(AdapterInfo, &dwBufLen) == ERROR_BUFFER_OVERFLOW) {
                free(AdapterInfo);
                AdapterInfo = (IP_ADAPTER_INFO*)malloc(dwBufLen);
                if (AdapterInfo == NULL) {
                    return result;
                }
            }

            if (GetAdaptersInfo(AdapterInfo, &dwBufLen) == NO_ERROR) {
                PIP_ADAPTER_INFO pAdapterInfo = AdapterInfo;
                do {
                    sprintf_s(mac_addr, sizeof(mac_addr), "%02X:%02X:%02X:%02X:%02X:%02X",
                        pAdapterInfo->Address[0], pAdapterInfo->Address[1],
                        pAdapterInfo->Address[2], pAdapterInfo->Address[3],
                        pAdapterInfo->Address[4], pAdapterInfo->Address[5]);

                    int64_t mac;
                    sscanf_s(mac_addr, "%llx", &mac);
                    result.push_back(MacAddress(mac));

                    pAdapterInfo = pAdapterInfo->Next;
                } while (pAdapterInfo);
            }
            free(AdapterInfo);

            return result;
        }

        inline static S cpu_id()
        {
            std::array<int, 4> cpuid_regs;
            __cpuid(cpuid_regs.data(), 0);
            auto a = std::to_string(cpuid_regs[1]);
            auto b = std::to_string(cpuid_regs[3]);
            return S(a.begin(), a.end()) + S(b.begin(), b.end());
        }

    } // namespace cpplib
} // namespace pensar_digital

#endif // SYSTEM_WINDOWS_HPP
