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

# ---------------------------------------------------------------------------
# ICU detection — use CMake's built-in FindICU module.
# On Linux (libicu-dev) it is found automatically.
# For Conan / custom installs pass -DICU_ROOT=/path/to/icu.
# ---------------------------------------------------------------------------
set(ICU_ROOT "" CACHE PATH "Path to ICU package root (contains include/ and lib/)")

find_package(ICU REQUIRED COMPONENTS uc i18n data io)

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
