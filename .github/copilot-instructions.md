# Copilot instructions for pensar

## Big picture
- This repo is the **cpplib** C++ utility library used across Pensar Digital projects; most code lives in cpp/src (headers only).
- Public types are under the `pensar_digital::cpplib` namespace (see cpp/src/s.hpp, cpp/src/code_util.hpp, cpp/src/bool.hpp).
- String handling is dual narrow/wide: `C`/`S` types and `W("...")` literal macro come from cpp/src/string_def.hpp; code should use these rather than raw `std::string` literals.
- ICU is a core dependency for Unicode utilities (see cpp/src/icu_util.hpp and cpp/src/cs.hpp for accent-insensitive comparisons).

## Build & test workflow
- Build is CMake-based; `CMAKE_CXX_STANDARD=23` and compile_commands.json is generated (see CMakeLists.txt).
- Tests are a custom framework; the `pensar_tests` target is built from cpp/src/test/*.cpp (see CMakeLists.txt).
- Test entrypoint: cpp/src/test/main.cpp calls `pensar_digital::unit_test::all_tests().run()`.

## Project-specific conventions
- Use `Bool` (tri-state) instead of `bool` when the API expects cpplib types (cpp/src/bool.hpp).
- Use `Result<T>` for error-aware return values (cpp/src/code_util.hpp) instead of exceptions in lightweight helpers.
- Logging uses `LOG(...)` macro from cpp/src/log.hpp; default log file path is Windows-style `C:\out\log.txt`.

## Custom test patterns
- Tests are registered via macros in unit_test/src/test.hpp: `TEST(name, is_enabled) ... TEST_END(name)` and `CHECK*` macros.
- New tests should live in cpp/src/test and include unit_test/src/test.hpp.

## External dependencies & integration
- ICU must be available and configured via `ICU_ROOT` (CMakeLists.txt); CMake auto-detects from Conan cache when `ICU_AUTO_DETECT=ON`.
- ICU headers/libraries are required: `icuuc`, `icui18n`, `icudata`, `icuio`.
