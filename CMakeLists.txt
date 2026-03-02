cmake_minimum_required(VERSION 3.20)

project(Pensar LANGUAGES CXX)

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

option(ICU_AUTO_DETECT "Search for ICU in the Conan cache or system paths" ON)
set(ICU_ROOT "" CACHE PATH "Path to ICU package root (contains include/ and lib/)")

# ---------------------------------------------------------------------------
# Cross-platform ICU detection
# 1. If ICU_ROOT is set explicitly, use it directly.
# 2. On Linux with system ICU packages (libicu-dev), detect /usr automatically.
# 3. Otherwise, search the Conan 2 cache (macOS, Windows, or any platform).
# ---------------------------------------------------------------------------
if(NOT ICU_ROOT AND ICU_AUTO_DETECT)
    # --- Linux system ICU from apt (libicu-dev) ---
    if(UNIX AND NOT APPLE)
        if(EXISTS "/usr/include/unicode/ucsdet.h")
            set(ICU_ROOT "/usr" CACHE PATH "System ICU from libicu-dev" FORCE)
            message(STATUS "Auto-detected system ICU at /usr")
        endif()
    endif()

    # --- Conan 2 cache search (cross-platform) ---
    if(NOT ICU_ROOT)
        set(_conan_root "$ENV{HOME}/.conan2")
        if(WIN32)
            set(_conan_root "$ENV{USERPROFILE}/.conan2")
        endif()
        if(EXISTS "${_conan_root}")
            # GLOB_RECURSE descends through all subdirs; Conan 2 layout:
            # .conan2/p/icu<hash>/p/include/unicode/ucsdet.h
            file(GLOB_RECURSE _icu_headers "${_conan_root}/p/icu*/p/include/unicode/ucsdet.h")
            if(NOT _icu_headers)
                # Fallback: broader search in case Conan layout varies
                file(GLOB_RECURSE _icu_headers "${_conan_root}/p/*/include/unicode/ucsdet.h")
            endif()
            if(_icu_headers)
                list(GET _icu_headers 0 _icu_header)
                get_filename_component(_icu_unicode_dir "${_icu_header}" DIRECTORY)
                get_filename_component(_icu_include_dir "${_icu_unicode_dir}" DIRECTORY)
                get_filename_component(_icu_root "${_icu_include_dir}" DIRECTORY)
                set(ICU_ROOT "${_icu_root}" CACHE PATH "Path to ICU root (auto-detected from Conan cache)" FORCE)
                message(STATUS "Auto-detected ICU_ROOT from Conan cache: ${ICU_ROOT}")
            endif()
        endif()
    endif()
endif()

if(ICU_ROOT)
    set(ICU_INCLUDE_DIR "${ICU_ROOT}/include")
    set(ICU_LIB_DIR "${ICU_ROOT}/lib")
endif()

if(NOT ICU_INCLUDE_DIR OR NOT EXISTS "${ICU_INCLUDE_DIR}/unicode/ucsdet.h")
    message(FATAL_ERROR "ICU include path not found. Set ICU_ROOT to the root that contains include/ and lib/.")
endif()

if(NOT ICU_LIB_DIR OR NOT EXISTS "${ICU_LIB_DIR}")
    message(FATAL_ERROR "ICU lib path not found. Set ICU_ROOT to the root that contains include/ and lib/.")
endif()

# find_library is platform-aware: finds .dylib on macOS, .so on Linux, .lib on Windows.
# Include versioned names (e.g. icuuc76) that Conan may produce on Windows.
find_library(ICU_UC_LIBRARY   NAMES icuuc   icuuc76   icuuc75   icuuc74   HINTS "${ICU_LIB_DIR}")
find_library(ICU_I18N_LIBRARY NAMES icui18n icui18n76 icui18n75 icui18n74 HINTS "${ICU_LIB_DIR}")
find_library(ICU_DATA_LIBRARY NAMES icudata icudata76 icudata75 icudata74 HINTS "${ICU_LIB_DIR}")
find_library(ICU_IO_LIBRARY   NAMES icuio   icuio76   icuio75   icuio74   HINTS "${ICU_LIB_DIR}")

if(NOT ICU_UC_LIBRARY OR NOT ICU_I18N_LIBRARY OR NOT ICU_DATA_LIBRARY OR NOT ICU_IO_LIBRARY)
    message(FATAL_ERROR "ICU libraries not found in ${ICU_LIB_DIR}. Ensure ICU_ROOT is correct.")
endif()

file(GLOB TEST_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/cpp/src/test/*.cpp")

add_executable(pensar_tests ${TEST_SOURCES})

target_include_directories(pensar_tests PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/cpp/src"
    "${CMAKE_CURRENT_SOURCE_DIR}/cpp/src/test"
    "${ICU_INCLUDE_DIR}"
    "${asio_SOURCE_DIR}/asio/include"
)

target_compile_definitions(pensar_tests PRIVATE
    ASIO_STANDALONE
)

target_link_libraries(pensar_tests PRIVATE
    Catch2::Catch2WithMain
    "${ICU_UC_LIBRARY}"
    "${ICU_I18N_LIBRARY}"
    "${ICU_DATA_LIBRARY}"
    "${ICU_IO_LIBRARY}"
)

# Windows needs additional system libraries for networking and shell APIs
if(WIN32)
    target_link_libraries(pensar_tests PRIVATE ws2_32 iphlpapi shell32)
endif()

enable_testing()
catch_discover_tests(pensar_tests)

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
    "${ICU_INCLUDE_DIR}"
    "${asio_SOURCE_DIR}/asio/include"
)

target_compile_definitions(contact_tests PRIVATE
    ASIO_STANDALONE
)

target_link_libraries(contact_tests PRIVATE
    Catch2::Catch2WithMain
    "${ICU_UC_LIBRARY}"
    "${ICU_I18N_LIBRARY}"
    "${ICU_DATA_LIBRARY}"
    "${ICU_IO_LIBRARY}"
)

# Windows needs additional system libraries for networking and shell APIs
if(WIN32)
    target_link_libraries(contact_tests PRIVATE ws2_32 iphlpapi shell32)
endif()

catch_discover_tests(contact_tests)

# ---------------------------------------------------------------------------
# contact_db_tests — integration tests for SOCI / PostgreSQL data layer
# ---------------------------------------------------------------------------
set(SOCI_ROOT "/opt/homebrew/opt/soci-pg" CACHE PATH "Path to SOCI install prefix")

find_library(SOCI_CORE_LIBRARY       NAMES soci_core       HINTS "${SOCI_ROOT}/lib")
find_library(SOCI_POSTGRESQL_LIBRARY NAMES soci_postgresql HINTS "${SOCI_ROOT}/lib")
find_library(PQ_LIBRARY              NAMES pq              HINTS "/opt/homebrew/lib/postgresql@18")

if(SOCI_CORE_LIBRARY AND SOCI_POSTGRESQL_LIBRARY AND PQ_LIBRARY)
    add_executable(contact_db_tests
        "${CMAKE_CURRENT_SOURCE_DIR}/contact/test/db_test.cpp"
    )

    target_include_directories(contact_db_tests PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/cpp/src"
        "${CMAKE_CURRENT_SOURCE_DIR}/contact"
        "${SOCI_ROOT}/include"
        "/opt/homebrew/include/postgresql@18"
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

    catch_discover_tests(contact_db_tests)
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
