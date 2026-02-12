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

option(ICU_AUTO_DETECT "Search for ICU in the Conan cache" ON)
set(ICU_ROOT "/Users/mg/.conan2/p/icu29a6cb2d4b90f/p" CACHE PATH "Path to ICU package root (contains include/ and lib/)")

if(NOT ICU_ROOT AND ICU_AUTO_DETECT)
    set(_conan_root "$ENV{HOME}/.conan2")
    if(EXISTS "${_conan_root}")
        execute_process(
            COMMAND /usr/bin/find "${_conan_root}" -path "*include/unicode/ucsdet.h" -print -quit
            OUTPUT_VARIABLE _icu_header
            OUTPUT_STRIP_TRAILING_WHITESPACE
        )
        if(_icu_header)
            get_filename_component(_icu_unicode_dir "${_icu_header}" DIRECTORY)
            get_filename_component(_icu_include_dir "${_icu_unicode_dir}" DIRECTORY)
            get_filename_component(_icu_root "${_icu_include_dir}" DIRECTORY)
            set(ICU_ROOT "${_icu_root}" CACHE PATH "Path to ICU package root (contains include/ and lib/)" FORCE)
        endif()
    endif()
endif()

if(ICU_ROOT)
    set(ICU_INCLUDE_DIR "${ICU_ROOT}/include")
    set(ICU_LIB_DIR "${ICU_ROOT}/lib")
endif()

if(NOT ICU_INCLUDE_DIR OR NOT EXISTS "${ICU_INCLUDE_DIR}/unicode/ucsdet.h")
    message(FATAL_ERROR "ICU include path not found. Set ICU_ROOT to the Conan package root that contains include/ and lib/.")
endif()

if(NOT ICU_LIB_DIR OR NOT EXISTS "${ICU_LIB_DIR}")
    message(FATAL_ERROR "ICU lib path not found. Set ICU_ROOT to the Conan package root that contains include/ and lib/.")
endif()

find_library(ICU_UC_LIBRARY NAMES icuuc HINTS "${ICU_LIB_DIR}")
find_library(ICU_I18N_LIBRARY NAMES icui18n HINTS "${ICU_LIB_DIR}")
find_library(ICU_DATA_LIBRARY NAMES icudata HINTS "${ICU_LIB_DIR}")
find_library(ICU_IO_LIBRARY NAMES icuio HINTS "${ICU_LIB_DIR}")

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

enable_testing()
catch_discover_tests(pensar_tests)

# ---------------------------------------------------------------------------
# contact_tests — tests for the contact module (person, email, phone)
# ---------------------------------------------------------------------------
file(GLOB CONTACT_TEST_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/contact/test/*.cpp")

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

catch_discover_tests(contact_tests)
