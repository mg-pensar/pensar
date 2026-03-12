cmake_minimum_required(VERSION 3.28)

# CMake 4.0+ changed cmake_parse_arguments() behaviour for empty keyword values.
# Setting CMP0174 to NEW silences warnings from FetchContent on CMake >= 3.30.
if(POLICY CMP0174)
  cmake_policy(SET CMP0174 NEW)
endif()

# ---------------------------------------------------------------------------
# Disable vcpkg's MSBuild integration for this project.
# vcpkg installs a global MSBuild hook (vcpkg.targets) that injects its own
# include / library paths and even a wildcard "*.lib" for every installed
# package.  This conflicts with Conan-managed dependencies (e.g. ICU 76 vs
# vcpkg's ICU 74).  Setting VcpkgEnabled=false via VS globals turns it off.
# ---------------------------------------------------------------------------
set(CMAKE_VS_GLOBALS "VcpkgEnabled=false")

project(Pensar LANGUAGES CXX)

# Enable C++20 module scanning when using a compatible generator (Ninja, VS 17.4+).
if(CMAKE_GENERATOR MATCHES "Ninja|Visual Studio")
  set(CMAKE_CXX_SCAN_FOR_MODULES ON)
endif()

Include(FetchContent)
FetchContent_Declare(
  Catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
  GIT_TAG        v3.7.1
)

FetchContent_Declare(
  asio
  GIT_REPOSITORY https://github.com/chriskohlhoff/asio.git
  GIT_TAG        asio-1-32-0
  CONFIGURE_COMMAND ""
  BUILD_COMMAND ""
)

FetchContent_MakeAvailable(Catch2 asio)
list(APPEND CMAKE_MODULE_PATH ${catch2_SOURCE_DIR}/extras)
include(Catch)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")

# ---------------------------------------------------------------------------
# Multi-config generators (Visual Studio, Xcode) append a per-config subdir
# (e.g. Debug/) to CMAKE_RUNTIME_OUTPUT_DIRECTORY.  Force all configs to
# output directly into CMAKE_BINARY_DIR so test runner paths are consistent.
# ---------------------------------------------------------------------------
foreach(cfg DEBUG RELEASE RELWITHDEBINFO MINSIZEREL)
  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_${cfg} "${CMAKE_BINARY_DIR}")
endforeach()

# ---------------------------------------------------------------------------
# MSVC-specific flags
# ---------------------------------------------------------------------------
if(MSVC)
  # /FS — force synchronous PDB writes so parallel /MP compilation doesn't
  #        fail with C1041 ("cannot open program database").
  add_compile_options(/FS)
  # NOMINMAX — prevent <windows.h> from defining min/max macros that
  #            collide with std::min / std::max / std::numeric_limits::max().
  add_compile_definitions(NOMINMAX)
endif()

# ---------------------------------------------------------------------------
# ICU detection — use CMake's built-in FindICU module.
# On Linux (libicu-dev) it is found automatically.
# For Conan / custom installs pass -DICU_ROOT=/path/to/icu.
# On macOS, Homebrew's icu4c is keg-only; auto-detect if ICU_ROOT is unset.
# ---------------------------------------------------------------------------
set(ICU_ROOT "" CACHE PATH "Path to ICU package root (contains include/ and lib/)")

if(NOT ICU_ROOT)
    if(APPLE)
        find_program(_HOMEBREW brew)
        if(_HOMEBREW)
            execute_process(
                COMMAND "${_HOMEBREW}" --prefix icu4c
                OUTPUT_VARIABLE _BREW_ICU_PREFIX
                OUTPUT_STRIP_TRAILING_WHITESPACE
                ERROR_QUIET
                RESULT_VARIABLE _BREW_RESULT
            )
            if(_BREW_RESULT EQUAL 0 AND EXISTS "${_BREW_ICU_PREFIX}")
                set(ICU_ROOT "${_BREW_ICU_PREFIX}" CACHE PATH
                    "Path to ICU (auto-detected from Homebrew)" FORCE)
                message(STATUS "ICU auto-detected via Homebrew: ${ICU_ROOT}")
            endif()
        endif()
    endif()
endif()

find_package(ICU REQUIRED COMPONENTS uc i18n data io)

# ---------------------------------------------------------------------------
# Conan single-profile fix: the CMakeDeps generator wraps every ICU library
# property in $<$<CONFIG:Release>:…>, so Debug / RelWithDebInfo / MinSizeRel
# builds see *no* ICU libraries at all.  On Windows with vcpkg installed the
# linker may silently fall back to vcpkg's older ICU (version mismatch).
#
# Fix: copy the Release-only IMPORTED properties to a config-agnostic form
# so the Conan ICU libraries are used in every configuration.
# ---------------------------------------------------------------------------
foreach(_icu_tgt ICU::uc ICU::i18n ICU::data ICU::io ICU::dt ICU::in ICU::tu ICU::test icu::icu icu_DEPS_TARGET
                  icu_ICU_data_DEPS_TARGET icu_ICU_dt_DEPS_TARGET
                  icu_ICU_uc_DEPS_TARGET icu_ICU_i18n_DEPS_TARGET
                  icu_ICU_in_DEPS_TARGET icu_ICU_io_DEPS_TARGET
                  icu_ICU_tu_DEPS_TARGET icu_ICU_test_DEPS_TARGET)
    if(TARGET ${_icu_tgt})
        # Strip CONFIG:Release guards from link/include/linkdir properties.
        # We intentionally skip INTERFACE_COMPILE_OPTIONS and
        # INTERFACE_COMPILE_DEFINITIONS because they contain nested genexps
        # ($<COMPILE_LANGUAGE:…>) that would be corrupted by the regex.
        foreach(_prop INTERFACE_LINK_LIBRARIES INTERFACE_INCLUDE_DIRECTORIES
                      INTERFACE_LINK_DIRECTORIES)
            get_target_property(_val ${_icu_tgt} ${_prop})
            if(_val)
                string(REGEX REPLACE "\\$<\\$<CONFIG:Release>:([^>]*)>" "\\1" _val_all "${_val}")
                set_property(TARGET ${_icu_tgt} PROPERTY ${_prop} "${_val_all}")
            endif()
        endforeach()
    endif()
endforeach()

# Also patch the per-library micro-targets Conan creates (CONAN_LIB::icu_*)
get_property(_all_targets DIRECTORY "${CMAKE_SOURCE_DIR}" PROPERTY IMPORTED_TARGETS)
foreach(_t ${_all_targets})
    if("${_t}" MATCHES "^CONAN_LIB::icu_")
        # Copy IMPORTED_LOCATION_RELEASE → IMPORTED_LOCATION (and IMPLIB)
        get_target_property(_loc ${_t} IMPORTED_LOCATION_RELEASE)
        if(_loc)
            set_target_properties(${_t} PROPERTIES IMPORTED_LOCATION "${_loc}")
        endif()
        get_target_property(_imp ${_t} IMPORTED_IMPLIB_RELEASE)
        if(_imp)
            set_target_properties(${_t} PROPERTIES IMPORTED_IMPLIB "${_imp}")
        endif()
        # Also patch INTERFACE_LINK_LIBRARIES
        get_target_property(_iface2 ${_t} INTERFACE_LINK_LIBRARIES)
        if(_iface2)
            string(REGEX REPLACE "\\$<\\$<CONFIG:Release>:([^>]*)>" "\\1" _iface2_all "${_iface2}")
            set_property(TARGET ${_t} PROPERTY INTERFACE_LINK_LIBRARIES "${_iface2_all}")
        endif()
    endif()
endforeach()

file(GLOB TEST_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/cpp/src/test/*.cpp")

add_executable(pensar_tests ${TEST_SOURCES})

target_include_directories(pensar_tests PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/cpp/src"
    "${CMAKE_CURRENT_SOURCE_DIR}/cpp/src/test"
    "${asio_SOURCE_DIR}/asio/include"
)

target_compile_definitions(pensar_tests PRIVATE
    ASIO_STANDALONE
)

target_link_libraries(pensar_tests PRIVATE
    Catch2::Catch2WithMain
    ICU::uc
    ICU::i18n
    ICU::data
    ICU::io
)

# Windows needs additional system libraries for networking and shell APIs
if(WIN32)
    target_link_libraries(pensar_tests PRIVATE ws2_32 iphlpapi shell32)
endif()

# ---------------------------------------------------------------------------
# Copy ICU shared libraries to the build output directory so test executables
# can find them at runtime without needing PATH modifications.
# ---------------------------------------------------------------------------
if(WIN32 AND icu_BIN_DIRS_RELEASE)
    file(GLOB _icu_dlls "${icu_BIN_DIRS_RELEASE}/*.dll")
    foreach(_dll ${_icu_dlls})
        get_filename_component(_dll_name "${_dll}" NAME)
        configure_file("${_dll}" "${CMAKE_BINARY_DIR}/${_dll_name}" COPYONLY)
    endforeach()
endif()

enable_testing()
catch_discover_tests(pensar_tests TEST_PREFIX "pensar::" DL_PATHS "${icu_BIN_DIRS_RELEASE}")

# ---------------------------------------------------------------------------
# contact_tests — tests for the contact module (person, email, phone)
# ---------------------------------------------------------------------------
# Separate non-DB and DB test sources so contact_tests doesn't need SOCI.
file(GLOB CONTACT_TEST_SOURCES CONFIGURE_DEPENDS
     "${CMAKE_CURRENT_SOURCE_DIR}/contact/test/person_test.cpp"
     "${CMAKE_CURRENT_SOURCE_DIR}/contact/test/e-mail_test.cpp")

add_executable(contact_tests ${CONTACT_TEST_SOURCES})

target_include_directories(contact_tests PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/cpp/src"
    "${CMAKE_CURRENT_SOURCE_DIR}/cpp/src/test"
    "${CMAKE_CURRENT_SOURCE_DIR}/contact"
    "${asio_SOURCE_DIR}/asio/include"
)

target_compile_definitions(contact_tests PRIVATE
    ASIO_STANDALONE
)

target_link_libraries(contact_tests PRIVATE
    Catch2::Catch2WithMain
    ICU::uc
    ICU::i18n
    ICU::data
    ICU::io
)

# Windows needs additional system libraries for networking and shell APIs
if(WIN32)
    target_link_libraries(contact_tests PRIVATE ws2_32 iphlpapi shell32)
endif()

catch_discover_tests(contact_tests TEST_PREFIX "contact::" DL_PATHS "${icu_BIN_DIRS_RELEASE}")

# ---------------------------------------------------------------------------
# contact_db_tests — integration tests for SOCI / PostgreSQL data layer
# ---------------------------------------------------------------------------
set(SOCI_ROOT "/opt/homebrew/opt/soci-pg" CACHE PATH "Path to SOCI install prefix")

find_library(SOCI_CORE_LIBRARY       NAMES soci_core       HINTS "${SOCI_ROOT}/lib" "/usr/lib" "/usr/lib/x86_64-linux-gnu")
find_library(SOCI_POSTGRESQL_LIBRARY NAMES soci_postgresql HINTS "${SOCI_ROOT}/lib" "/usr/lib" "/usr/lib/x86_64-linux-gnu")
find_library(PQ_LIBRARY              NAMES pq              HINTS "/opt/homebrew/lib/postgresql@18" "/usr/lib" "/usr/lib/x86_64-linux-gnu")

# Discover libpq include path via pkg-config (needed on Linux where
# libpq-fe.h lives under /usr/include/postgresql).
find_package(PkgConfig QUIET)
if(PkgConfig_FOUND)
    pkg_check_modules(LIBPQ QUIET libpq)
endif()

if(SOCI_CORE_LIBRARY AND SOCI_POSTGRESQL_LIBRARY AND PQ_LIBRARY)
    add_executable(contact_db_tests
        "${CMAKE_CURRENT_SOURCE_DIR}/contact/test/db_test.cpp"
    )

    target_include_directories(contact_db_tests PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/cpp/src"
        "${CMAKE_CURRENT_SOURCE_DIR}/contact"
        "${SOCI_ROOT}/include"
        "/opt/homebrew/include/postgresql@18"
        ${LIBPQ_INCLUDE_DIRS}
    )

    target_link_libraries(contact_db_tests PRIVATE
        Catch2::Catch2WithMain
        "${SOCI_CORE_LIBRARY}"
        "${SOCI_POSTGRESQL_LIBRARY}"
        "${PQ_LIBRARY}"
    )

    # Set RPATH so dynamic SOCI libs are found at runtime
    set_target_properties(contact_db_tests PROPERTIES
        BUILD_RPATH "${SOCI_ROOT}/lib;/opt/homebrew/lib/postgresql@18"
    )

    catch_discover_tests(contact_db_tests TEST_PREFIX "contact_db::")
else()
    message(STATUS "SOCI or libpq not found — skipping contact_db_tests target")
    if(NOT SOCI_CORE_LIBRARY)
        message(STATUS "  SOCI_CORE_LIBRARY not found in ${SOCI_ROOT}/lib")
    endif()
    if(NOT SOCI_POSTGRESQL_LIBRARY)
        message(STATUS "  SOCI_POSTGRESQL_LIBRARY not found in ${SOCI_ROOT}/lib")
    endif()
    if(NOT PQ_LIBRARY)
        message(STATUS "  PQ_LIBRARY (libpq) not found")
    endif()
endif()
