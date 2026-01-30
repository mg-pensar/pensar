// license: MIT (https://opensource.org/licenses/MIT)
#ifndef BINARY_BUFFER_HPP_INCLUDED
#define BINARY_BUFFER_HPP_INCLUDED

#include <vector>
#include <span>
#include <cstdio> // for FILE operations
#include <cstring> // for std::memcpy
#include <concepts>
#include <type_traits>
#include <expected>
#include <print>
#include <cstddef> // for std::byte
#include <string_view>

namespace pensar_digital
{
    namespace cpplib
    {
        // ---------------------------------------------------------------------------
        // Concept: BinarySerializable
        // ---------------------------------------------------------------------------
        // Ensures the type T has a bytes() method returning a span of const byte.
        // Also ensures the type is safe for raw memory operations.
        // ---------------------------------------------------------------------------
        template <typename T>
        concept BinarySerializable = requires(const T& t) {
            { t.bytes() } -> std::convertible_to<std::span<const std::byte>>;
            requires std::is_trivially_copyable_v<T>;
        };

        // ---------------------------------------------------------------------------
        // Class: BinaryBuffer
        // ---------------------------------------------------------------------------
        // A high-performance binary buffer that avoids std::iostream overhead.
        // Uses C++23 features like 'deduced this' and 'std::expected'.
        // ---------------------------------------------------------------------------
        class BinaryBuffer {
        private:
            std::vector<std::byte> buffer;
            size_t write_pos = 0;
            size_t read_pos = 0;

        public:
            explicit BinaryBuffer(size_t reserve = 4096) {
                buffer.reserve(reserve);
            }

            // --- View Data ---

            [[nodiscard]] std::span<const std::byte> data() const noexcept {
                return std::span{ buffer.data(), write_pos };
            }

            [[nodiscard]] size_t size() const noexcept { return write_pos; }

            void clear() noexcept {
                write_pos = 0;
                read_pos = 0;
            }

            // ======================================================================
            // WRITE METHODS
            // ======================================================================

            // Core write: copies a span of bytes into the buffer.
            auto write(this auto&& self, std::span<const std::byte> src) -> decltype(auto) {
                const size_t required = self.write_pos + src.size();

                // Check and resize buffer if necessary
                if (required > self.buffer.size()) {
                    self.buffer.resize(required);
                }

                // Copy memory directly (avoids virtual calls/overhead)
                std::memcpy(self.buffer.data() + self.write_pos, src.data(), src.size());
                self.write_pos += src.size();

                return self;
            }

            // Overload for writable byte spans to avoid POD overload selection.
            auto write(this auto&& self, std::span<std::byte> src) -> decltype(auto) {
                return self.write(std::span<const std::byte>(src.data(), src.size()));
            }

            // Write any object that satisfies BinarySerializable (e.g., your Person class)
            template <BinarySerializable T>
            auto write(this auto&& self, const T& obj) -> decltype(auto) {
                return self.write(obj.bytes());
            }

            // Fallback for trivial types (int, float, etc.) that don't have .bytes()
            template <typename T>
            auto write(this auto&& self, const T& pod) -> decltype(auto) 
                requires std::is_trivially_copyable_v<T> && (!BinarySerializable<T>) &&
                         (!std::is_same_v<std::remove_cvref_t<T>, std::span<std::byte>>) &&
                         (!std::is_same_v<std::remove_cvref_t<T>, std::span<const std::byte>>)
            {
                return self.write(std::as_bytes(std::span{ &pod, 1 }));
            }

            // Save entire buffer to disk (Binary Mode)
            std::expected<void, std::string_view> save_to_file(std::string_view filename) const {
                // "wb" is crucial for Windows to prevent newline translation
                FILE* f = fopen(std::string(filename).c_str(), "wb");
                if (!f) return std::unexpected("Failed to open file for writing");

                size_t written = fwrite(buffer.data(), sizeof(std::byte), write_pos, f);
                fclose(f);

                if (written != write_pos) {
                    return std::unexpected("Failed to write all bytes to disk");
                }
                return {};
            }

            // ======================================================================
            // READ METHODS
            // ======================================================================

            // Load entire file into buffer
            std::expected<void, std::string_view> load_from_file(std::string_view filename) {
                FILE* f = fopen(std::string(filename).c_str(), "rb");
                if (!f) return std::unexpected("Failed to open file for reading");

                // Get file size
                fseek(f, 0, SEEK_END);
                long file_size = ftell(f);
                fseek(f, 0, SEEK_SET);

                if (file_size < 0) {
                    fclose(f);
                    return std::unexpected("Could not determine file size");
                }

                // Resize buffer to fit file contents
                buffer.resize(static_cast<size_t>(file_size));

                // Read bytes
                size_t read_bytes = fread(buffer.data(), sizeof(std::byte), static_cast<size_t>(file_size), f);
                fclose(f);

                if (read_bytes != static_cast<size_t>(file_size)) {
                    return std::unexpected("Read mismatch: read less bytes than expected");
                }

                // Reset positions
                write_pos = static_cast<size_t>(file_size);
                read_pos = 0;
                return {};
            }

            // Core read: copies buffer bytes into the destination span
            auto read(this auto&& self, std::span<std::byte> dest) -> decltype(auto) {
                // Check bounds
                if (self.read_pos + dest.size() > self.buffer.size()) {
                    std::println("Error: Buffer underflow while reading!");
                    return self;
                }

                // Copy from buffer to destination
                std::memcpy(dest.data(), self.buffer.data() + self.read_pos, dest.size());
                self.read_pos += dest.size();

                return self;
            }

            // Read into a BinarySerializable object
            // Note: This overwrites the memory of 'obj'.
            template <BinarySerializable T>
            auto read(this auto&& self, T& obj) -> decltype(auto) {
                // View the object as a writable byte span
                auto dest_span = std::as_writable_bytes(std::span{ &obj, 1 });
                return self.read(dest_span);
            }

            // Read into a trivial type (int, float, etc.)
            template <typename T>
            auto read(this auto&& self, T& pod) -> decltype(auto) 
                requires std::is_trivially_copyable_v<T> && (!BinarySerializable<T>) &&
                         (!std::is_same_v<std::remove_cvref_t<T>, std::span<std::byte>>) &&
                         (!std::is_same_v<std::remove_cvref_t<T>, std::span<const std::byte>>)
            {
                auto dest_span = std::as_writable_bytes(std::span{ &pod, 1 });
                return self.read(dest_span);
            }
        };
    } // namespace cpplib
} // namespace pensar_digital

#endif // BINARY_BUFFER_HPP_INCLUDED