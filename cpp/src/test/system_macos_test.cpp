// author : Mauricio Gomes
// license: MIT (https://opensource.org/licenses/MIT)

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>

#include "../system.hpp"

namespace pensar_digital::cpplib
{
    using Catch::Matchers::ContainsSubstring;

    // ── Platform constants ────────────────────────────────────────────

    TEST_CASE("System OS name is macOS", "[system][macos]")
    {
        CHECK(Sys::OS_NAME == W("macOS"));
    }

    TEST_CASE("System OS type is macOS", "[system][macos]")
    {
        CHECK(Sys::OS == OSType::macOS);
    }

    TEST_CASE("System line feed is LF", "[system][macos]")
    {
        CHECK(Sys::LF == W("\n"));
    }

    TEST_CASE("System path separator is forward slash", "[system][macos]")
    {
        CHECK(Sys::PATH_SEPARATOR == W('/'));
    }

    TEST_CASE("System max name length is 255 (APFS)", "[system][macos]")
    {
        CHECK(Sys::MAX_NAME_LENGTH == 255);
    }

    TEST_CASE("System max path is 1024", "[system][macos]")
    {
        CHECK(Sys::MAX_PATH == 1024);
    }

    // ── Endianness ────────────────────────────────────────────────────

    TEST_CASE("System endianness is consistent", "[system]")
    {
        auto e = Sys::endianess();
        REQUIRE((e == std::endian::little || e == std::endian::big));
        S name = Sys::endianess_name();
        if (e == std::endian::little)
            CHECK(name == W("Little Endian"));
        else
            CHECK(name == W("Big Endian"));
    }

    // ── File / path name validation ──────────────────────────────────

    TEST_CASE("System valid file names", "[system][macos]")
    {
        CHECK(Sys::is_valid_file_name(W("hello.txt")));
        CHECK(Sys::is_valid_file_name(W("readme")));
        CHECK(Sys::is_valid_file_name(W("a")));
    }

    TEST_CASE("System invalid file names", "[system][macos]")
    {
        CHECK_FALSE(Sys::is_valid_file_name(W("")));           // empty
        CHECK_FALSE(Sys::is_valid_file_name(W(".hidden")));    // starts with dot
        CHECK_FALSE(Sys::is_valid_file_name(W("a/b")));       // contains separator
        S with_null = W("a");
        with_null += W('\0');
        with_null += W("b");
        CHECK_FALSE(Sys::is_valid_file_name(with_null));       // embedded null
    }

    TEST_CASE("System valid paths", "[system][macos]")
    {
        CHECK(Sys::is_valid_path(W("/usr/local/bin")));
        CHECK(Sys::is_valid_path(W("relative/path")));
    }

    TEST_CASE("System invalid paths", "[system][macos]")
    {
        CHECK_FALSE(Sys::is_valid_path(W("")));                // empty
        CHECK_FALSE(Sys::is_valid_path(W(".hidden_dir")));     // starts with dot
    }

    // ── MAC addresses ────────────────────────────────────────────────

    TEST_CASE("mac_addresses returns at least one address", "[system][macos][network]")
    {
        auto macs = mac_addresses();
        REQUIRE_FALSE(macs.empty());

        SECTION("Each MAC address is non-zero")
        {
            for (const auto& m : macs)
            {
                int64_t val = static_cast<int64_t>(m);
                CHECK(val != 0);
            }
        }

        SECTION("MAC addresses convert to colon-separated string of length 17")
        {
            for (const auto& m : macs)
            {
                std::string s = static_cast<std::string>(m);
                // Format: xx:xx:xx:xx:xx:xx  →  17 characters
                CHECK(s.size() == 17);
                // Five colons expected
                auto colons = std::count(s.begin(), s.end(), ':');
                CHECK(colons == 5);
            }
        }

        SECTION("Consecutive calls return the same set of addresses")
        {
            auto macs2 = mac_addresses();
            REQUIRE(macs.size() == macs2.size());
            for (size_t i = 0; i < macs.size(); ++i)
            {
                CHECK(static_cast<int64_t>(macs[i]) == static_cast<int64_t>(macs2[i]));
            }
        }
    }

    // ── CPU ID ───────────────────────────────────────────────────────

    TEST_CASE("cpu_id returns a non-empty string", "[system][macos][cpu]")
    {
        S id = cpu_id();
        REQUIRE_FALSE(id.empty());

        SECTION("cpu_id is deterministic")
        {
            CHECK(cpu_id() == id);
        }

        SECTION("cpu_id contains meaningful content")
        {
            // On Apple Silicon expect "Mac" somewhere (e.g. "Mac15,9");
            // on Intel expect brand string with "Intel" or similar.
            // At minimum the string should be longer than 3 characters.
            CHECK(id.size() > 3);
        }
    }

} // namespace pensar_digital::cpplib
