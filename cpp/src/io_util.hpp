// author : Mauricio Gomes
// license: MIT (https://opensource.org/licenses/MIT)
#ifndef IO_UTIL_HPP
#define IO_UTIL_HPP

#include <span>
#include <cstdio> // for FILE operations
#include <cstring> // for std::memcpy
#include <cstddef> // for std::byte
#include <expected>
#include <string_view>
#include <vector>

#include "multiplatform.hpp"
#include "defines.hpp"
#include "s.hpp"
#include "memory.hpp"
#include "constant.hpp"
#include "concept.hpp"
#include "icu_util.hpp"
#include "log.hpp"
#include "error.hpp"

#include "code_util.hpp"

// Detects and includes platform-specific implementation header.
// Examples:
// #include "windows/io_util_windows.hpp"
// #include "linux/io_util_linux.hpp" 
// #include "android/io_util_android.hpp" or other platforms
#include INCLUDE(io_util)



namespace pensar_digital 
{
    namespace cpplib 
    {

        using LINE_HANDLER = void(*)(const int64_t line_count, const S& line);

        Result<S> get_exe_full_path();


        // Save entire buffer to disk (Binary Mode)
        inline std::expected<void, std::string_view> save_to_file (std::string_view filename, const std::span<const std::byte> buffer, size_t offset = 0) 
        {
            // "wb" is crucial for Windows to prevent newline translation
            FILE* f = fopen(std::string(filename).c_str(), "wb");
            if (!f) return std::unexpected("Failed to open file for writing");

            size_t written = fwrite(buffer.data() + offset, sizeof(std::byte), buffer.size() - offset, f);
            fclose(f);

            if (written != buffer.size() - offset) 
            {
                return std::unexpected("Failed to write all bytes to disk");
            }
            return {};
        }

        // Load entire file into buffer
        inline std::expected<std::vector<std::byte>, std::string_view> load_from_file(std::string_view filename) 
        {
            FILE* f = fopen(std::string(filename).c_str(), "rb");
            if (!f) return std::unexpected("Failed to open file for reading");

            // Get file size
            fseek(f, 0, SEEK_END);
            long file_size = ftell(f);
            fseek(f, 0, SEEK_SET);

            if (file_size < 0) 
            {
                fclose(f);
                return std::unexpected("Could not determine file size");
            }

            // Resize buffer to fit file contents
            std::vector<std::byte> buffer(static_cast<size_t>(file_size));

            // Read bytes
            size_t read_bytes = fread(buffer.data(), sizeof(std::byte), static_cast<size_t>(file_size), f);
            fclose(f);

            if (read_bytes != static_cast<size_t>(file_size)) 
            {
                return std::unexpected("Read mismatch: read less bytes than expected");
            }

            return buffer;
        }


    }  // namespace cpplib
}  // namespace pensar_digital

#endif // IO_UTIL_HPP

