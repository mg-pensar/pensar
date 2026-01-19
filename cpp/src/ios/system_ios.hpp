// author : Mauricio Gomes
// license: MIT (https://opensource.org/licenses/MIT)

#ifndef SYSTEM_IOS_HPP
#define SYSTEM_IOS_HPP

// Specialization for iOS
template<> 
class System<OS::IOS> : public BaseSystem
        {
        public:
            inline static const S LINE_FEED = W("\n");
            inline static constexpr size_t get_max_name_length() noexcept { return 255; /* APFS */ }
            
            inline static constexpr size_t get_max_path() noexcept
            {
                #define DEFAULT_IOS_MAX_PATH 4096
                return DEFAULT_IOS_MAX_PATH;
            }
            inline static constexpr C path_separator() noexcept { return W('/'); }
            
            inline static bool is_valid_file_name(const S& file_name)
            {
                return is_name_valid_common(file_name) && file_name.find(path_separator()) == S::npos && file_name.find('\0') == S::npos;
            }

            inline static bool is_valid_path(const S& path_name)
            {
                return is_name_valid_common(path_name) && path_name.find(W('\0')) == S::npos;
            }

            inline static std::vector<MacAddress> mac_addresses()
            {
                // TODO: Implement iOS-specific MAC address retrieval
                return std::vector<MacAddress>();
            }

            inline static S cpu_id()
            {
                // TODO: Implement iOS-specific CPU ID retrieval
                return W("");
            }
        };

#endif // SYSTEM_IOS_HPP
