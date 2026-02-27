## Plan: Transition cpplib from `#include` to C++20/23 Modules (Revised — Detailed Steps 0a, 0b)

**TL;DR** — Migrate the header-only cpplib library to named C++ modules in ~13 incremental steps, bottom-up following the dependency graph. Before any changes, validate the current codebase builds and passes tests on all three target platforms (macOS, Windows, Linux Ubuntu Server). Macros (`W()`, `LOG()`, `INCLUDE()`, `data.hpp` verification macros) are handled first as prerequisite refactors since modules cannot export macros. The target module structure is hybrid: `import cpplib;` for everything, or `import cpplib.object;` for specific parts, implemented via module partitions. Targets Clang 18+ (Homebrew on macOS, apt on Ubuntu) and MSVC (latest) on Windows.

---

### Step 0a. Windows — Build & Test with MSVC

#### 0a.1 — Install Required Software

**Visual Studio 2022 (Community — free)**
1. Download from https://visualstudio.microsoft.com/downloads/ — pick "Community" edition.
2. Run the installer. When the **Workloads** screen appears, check **"Desktop development with C++"**.
3. On the right-hand side, ensure these are selected (they usually are by default):
   - "MSVC v143 – VS 2022 C++ x64/x86 build tools (Latest)"
   - "Windows 11 SDK" (or Windows 10 SDK)
   - "C++ CMake tools for Windows"
4. Click **Install**. This takes 10–30 minutes depending on your internet speed.
5. After installation, open **"Developer Command Prompt for VS 2022"** from the Start menu. This is a special terminal that has all MSVC tools (`cl.exe`, `cmake`, `nmake`, etc.) pre-configured in the `PATH`. **Always use this terminal** for the commands below, not regular PowerShell or CMD.

**Git for Windows**
1. Download from https://git-scm.com/download/win.
2. Install with defaults. This gives you `git` in the terminal.

**Conan 2 (C++ package manager — needed for ICU)**
1. Open Developer Command Prompt.
2. Check if Python is available: `python --version`. If not installed:
   - Download Python from https://www.python.org/downloads/ (3.10+ recommended).
   - During installation, **check "Add python.exe to PATH"**.
3. Install Conan:
   ```
   pip install conan
   ```
4. Verify: `conan --version` — should print `Conan version 2.x.x`.
5. Create a default Conan profile:
   ```
   conan profile detect
   ```
   This auto-detects your MSVC compiler and creates `~/.conan2/profiles/default`.

#### 0a.2 — Install ICU via Conan

ICU is the Unicode library the project depends on. On macOS you already have it in your Conan cache; now you need it on Windows.

1. In Developer Command Prompt, run:
   ```
   conan install --requires="icu/76.1" --build=missing
   ```
   This downloads and builds ICU. It may take 5–15 minutes the first time.

2. Find where Conan installed ICU:
   ```
   conan list "icu/76.1:*"
   ```
   This shows the package ID. The install path is typically:
   ```
   C:\Users\<YourUser>\.conan2\p\icu<hash>\p
   ```
   Inside that folder you'll find `include\unicode\ucsdet.h` and `lib\icuuc.lib` etc.

3. Note this path — you'll pass it as `ICU_ROOT` to CMake. For example:
   ```
   set ICU_ROOT=C:\Users\mg\.conan2\p\icu1a2b3c4d5e\p
   ```

#### 0a.3 — Clone the Repository

```
cd C:\Users\<YourUser>\Projects
git clone <your-repo-url> pensar
cd pensar
```

(If you transfer the code via USB/zip instead of git, just extract it and `cd` into the folder.)

#### 0a.4 — Fix CMakeLists.txt for Cross-Platform ICU Detection

The current CMakeLists.txt has macOS-specific paths hardcoded. Before building on Windows, you need to patch the ICU auto-detection. Open `CMakeLists.txt` in any text editor and make these changes:

**Replace the ICU auto-detect block** (lines ~33–50) with a cross-platform version:

```cmake
# Replace the hardcoded macOS default:
# OLD: set(ICU_ROOT "/Users/mg/.conan2/p/icu29a6cb2d4b90f/p" CACHE PATH ...)
# NEW:
set(ICU_ROOT "" CACHE PATH "Path to ICU package root (contains include/ and lib/)")

if(NOT ICU_ROOT AND ICU_AUTO_DETECT)
    # Search in the user's Conan 2 cache, cross-platform
    set(_conan_root "$ENV{HOME}/.conan2")
    if(WIN32)
        set(_conan_root "$ENV{USERPROFILE}/.conan2")
    endif()
    if(EXISTS "${_conan_root}")
        file(GLOB_RECURSE _icu_headers "${_conan_root}/p/icu*/include/unicode/ucsdet.h")
        if(_icu_headers)
            list(GET _icu_headers 0 _icu_header)
            get_filename_component(_icu_unicode_dir "${_icu_header}" DIRECTORY)
            get_filename_component(_icu_include_dir "${_icu_unicode_dir}" DIRECTORY)
            get_filename_component(_icu_root "${_icu_include_dir}" DIRECTORY)
            set(ICU_ROOT "${_icu_root}" CACHE PATH "Path to ICU root" FORCE)
            message(STATUS "Auto-detected ICU_ROOT: ${ICU_ROOT}")
        endif()
    endif()
endif()
```

This replaces the Unix-only `/usr/bin/find` command with CMake's cross-platform `file(GLOB_RECURSE ...)`.

On Windows, the ICU libraries are `.lib` files (e.g., `icuuc.lib`) instead of `.so`/`.dylib`. The `find_library` calls already handle this correctly since CMake's `find_library` is platform-aware.

#### 0a.5 — Fix the Linux io_util File (it would block compilation)

The file `cpp/src/linux/io_util_linux.cpp` erroneously includes Windows headers (`<winnt.h>`, `<handleapi.h>`, `<fileapi.h>`, `<minwindef.h>`). While this file isn't compiled on Windows (it's Linux-specific), the `file(GLOB ...)` in CMakeLists.txt for `pensar_tests` picks up **all** `.cpp` files in `cpp/src/test/`, not the Linux `.cpp` files. Verify this isn't a problem:

- Check that `cpp/src/linux/io_util_linux.cpp` is **not** in `cpp/src/test/` — it's in `cpp/src/linux/`, so it won't be globbed. **No action needed.**

However, if any test file includes `io_util.hpp` (which uses the `INCLUDE()` macro), the platform-specific header `windows/io_util_windows.hpp` will be pulled in on Windows. Make sure `defines.hpp` correctly identifies the platform. Check that on MSVC, the `WINDOWS` define is active and the `#include INCLUDE(io_util)` expands to the Windows header.

#### 0a.6 — Configure with CMake

In Developer Command Prompt, from the `pensar` directory:

```
cmake -S . -B build -DICU_ROOT=C:\Users\mg\.conan2\p\icu1a2b3c4d5e\p
```

Replace the `ICU_ROOT` path with the actual path you found in step 0a.2.

**What to expect:**
- CMake downloads Catch2 and Asio via FetchContent (requires internet, takes 1–2 minutes first time).
- CMake finds the ICU libraries in the path you provided.
- `contact_db_tests` will be skipped (SOCI/PostgreSQL not installed — that's fine).
- You should see: `Configuring done` and `Generating done`.

**If CMake fails:**
- `"ICU include path not found"` → Double-check your `ICU_ROOT` path. It should contain an `include` subfolder with `unicode/ucsdet.h` inside.
- `"ICU libraries not found"` → Look inside `ICU_ROOT/lib`. The files should be named `icuuc.lib`, `icui18n.lib`, `icudata.lib`, `icuio.lib` (on Windows, Conan may name them slightly differently like `icuuc76.lib` — if so, update the `find_library` `NAMES` in CMakeLists.txt to include the versioned names).

#### 0a.7 — Build

```
cmake --build build --target pensar_tests --config Release
```

(`--config Release` is needed for multi-config generators like Visual Studio.)

**Expected issues and fixes:**

| Error | Cause | Fix |
|---|---|---|
| `<experimental/filesystem>` not found | Old include in `cpp/src/linux/io_util_linux.hpp` — but this file shouldn't be included on Windows | Verify `INCLUDE()` macro resolves to `windows/io_util_windows.hpp` |
| `W()` related warnings | MSVC may warn about macro expansions | Non-blocking — note for later |
| `#pragma comment(lib, "IPHLPAPI.lib")` | `system_windows.hpp` uses this | Should work automatically with MSVC; if linker errors about `GetAdaptersInfo`, add `IPHLPAPI.lib` to `target_link_libraries` in CMakeLists.txt |
| Link errors for `Shell32.lib` or `Ws2_32.lib` | Windows API calls in `sys_user_info_windows.hpp` and `system_windows.hpp` | Add to CMakeLists.txt: `if(WIN32) target_link_libraries(pensar_tests PRIVATE ws2_32 iphlpapi shell32) endif()` |

#### 0a.8 — Run Tests

```
cd build
ctest --output-on-failure -C Release
```

(`-C Release` is required on Windows with multi-config generators.)

**Expected issues:**
- Tests that write files to hardcoded Unix paths (e.g., `./log.txt`, `/tmp/...`) may fail — check test output and fix paths to use `std::filesystem::temp_directory_path()` or relative paths.
- ICU runtime DLLs must be findable. If you get errors like `"icuuc76.dll not found"`, copy the DLLs from `ICU_ROOT/bin` to the build output directory, or add `ICU_ROOT/bin` to your `PATH`:
  ```
  set PATH=%ICU_ROOT%\bin;%PATH%
  ctest --output-on-failure -C Release
  ```

#### 0a.9 — Record Results

Document every fix you made. Commit each platform fix separately with a message like `fix: Windows MSVC build — add link libraries` or `fix: Windows — ICU DLL runtime path`. All `pensar_tests` and `contact_tests` must pass before moving on.

---

### Step 0b. Linux Ubuntu Server — Build & Test

#### 0b.1 — Get Access to an Ubuntu Server

You have several options (pick one):

**Option A — Virtual Machine on your Mac (easiest to start)**
1. Install UTM (free, native Apple Silicon): `brew install --cask utm`
2. Download the Ubuntu Server 24.04 LTS ARM64 ISO from https://ubuntu.com/download/server/arm
3. In UTM, click "Create a New Virtual Machine" → "Virtualize" → "Linux"
4. Select the downloaded ISO. Give it at least 4 GB RAM and 20 GB disk.
5. Walk through the Ubuntu installer (pick defaults for everything, create a username/password).
6. After installation, you'll have a terminal. All commands below happen here.

**Option B — Cloud VM (faster if you have an account)**
- On AWS: Launch an `t4g.medium` (ARM) or `t3.medium` (x86) instance with "Ubuntu Server 24.04 LTS" AMI. Connect via SSH.
- On Azure/GCP: Similar — create a small Ubuntu 24.04 VM and SSH in.
- On DigitalOcean: Create a $6/month droplet with Ubuntu 24.04.

**Option C — WSL on a Windows machine (if you have one)**
1. Open PowerShell as Administrator.
2. Run: `wsl --install -d Ubuntu-24.04`
3. Restart if prompted. Open "Ubuntu" from the Start menu.

**Option D — GitHub Codespace or CI (for automation later)**
- We'll set this up in Step 0d — skip for now.

#### 0b.2 — Install Build Tools

Once you're at an Ubuntu terminal (via any option above), run these commands:

```bash
# Update package lists
sudo apt update

# Install essential build tools: GCC, G++, make
sudo apt install -y build-essential

# Install CMake (need 3.20+; Ubuntu 24.04 ships 3.28+)
sudo apt install -y cmake

# Verify versions
gcc --version      # Should show 13.x or newer
cmake --version    # Should show 3.28.x or newer
```

#### 0b.3 — Install ICU Development Libraries

On Ubuntu, ICU is available as a system package — no need for Conan:

```bash
# Install ICU development headers and libraries
sudo apt install -y libicu-dev

# Verify installation
icu-config --version    # Should print something like 74.2
ls /usr/include/unicode/ucsdet.h   # Should exist
ls /usr/lib/*/libicuuc.so          # Should exist
```

The libraries are installed to `/usr/lib/x86_64-linux-gnu/` (on x86) or `/usr/lib/aarch64-linux-gnu/` (on ARM).

#### 0b.4 — Install Git and Clone the Repository

```bash
# Install git
sudo apt install -y git

# Clone your repo (replace URL with your actual repo URL)
cd ~
git clone <your-repo-url> pensar
cd pensar
```

(Or transfer the code via `scp` from your Mac: `scp -r ~/pensar user@ubuntu-ip:~/pensar`)

#### 0b.5 — Fix CMakeLists.txt for System ICU on Linux

The current CMakeLists.txt looks for ICU in the Conan cache. On Ubuntu with system ICU packages, the headers are in `/usr/include` and libraries are in `/usr/lib`. You need to tell CMake where to find them.

**Option A — Set ICU_ROOT explicitly:**
On Ubuntu with system packages, ICU files are in `/usr` (headers in `/usr/include/unicode/`, libs in `/usr/lib/...`):

```bash
cmake -S . -B build -DICU_ROOT=/usr
```

**Option B — If that doesn't work** (because `find_library` can't find libs in `/usr/lib`), pass the lib directory explicitly. First find the exact path:

```bash
# Find where libicuuc is
dpkg -L libicu-dev | grep libicuuc.so
# Output example: /usr/lib/x86_64-linux-gnu/libicuuc.so
```

Then pass it to CMake:

```bash
cmake -S . -B build \
  -DICU_ROOT=/usr \
  -DCMAKE_LIBRARY_PATH=/usr/lib/x86_64-linux-gnu
```

(Replace `x86_64-linux-gnu` with `aarch64-linux-gnu` if on ARM.)

**Option C — Patch CMakeLists.txt properly** (recommended — makes it work automatically): Add this block right before the `find_library` calls in `CMakeLists.txt`:

```cmake
# On Linux with system ICU packages, help find_library locate them
if(UNIX AND NOT APPLE)
    if(NOT ICU_ROOT OR ICU_ROOT STREQUAL "")
        # System ICU from apt (libicu-dev)
        if(EXISTS "/usr/include/unicode/ucsdet.h")
            set(ICU_ROOT "/usr" CACHE PATH "System ICU" FORCE)
            message(STATUS "Using system ICU from /usr")
        endif()
    endif()
endif()
```

#### 0b.6 — Configure with CMake

```bash
cd ~/pensar
cmake -S . -B build
```

**What to expect:**
- CMake downloads Catch2 and Asio (requires internet).
- CMake finds the ICU libraries.
- `contact_db_tests` will be skipped (no SOCI/PostgreSQL — that's fine).
- You should see: `-- Configuring done` and `-- Generating done`.

**If CMake fails with ICU errors:**
```bash
# Debug: check what CMake found
cmake -S . -B build -DICU_ROOT=/usr 2>&1 | grep -i icu
```

If `find_library` still can't find the libs, explicitly pass the library directory:
```bash
cmake -S . -B build \
  -DICU_ROOT=/usr \
  -DCMAKE_PREFIX_PATH=/usr
```

#### 0b.7 — Build

```bash
cmake --build build --target pensar_tests -j$(nproc)
```

(`-j$(nproc)` uses all CPU cores for parallel compilation. `nproc` returns the number of cores.)

**Expected issues and fixes:**

| Error | Cause | Fix |
|---|---|---|
| `<experimental/filesystem>` in `io_util_linux.hpp` | Old pre-C++17 include; C++23 uses `<filesystem>` | Change `#include <experimental/filesystem>` to `#include <filesystem>` in `io_util_linux.hpp` |
| Windows headers in `io_util_linux.cpp` (`<winnt.h>`, `<handleapi.h>`, `<fileapi.h>`, `<minwindef.h>`) | These files don't exist on Linux | Remove these 4 includes from `io_util_linux.cpp`. They're clearly erroneous — the file is in the `linux/` folder. Wrap them in `#ifdef _WIN32` if unsure. |
| `io_util_linux.cpp` is not compiled | The `file(GLOB ...)` in CMakeLists.txt only picks up `cpp/src/test/*.cpp`, not `cpp/src/linux/*.cpp` | If `io_util_linux.cpp` contains function definitions needed at link time, add it as a source file: `target_sources(pensar_tests PRIVATE cpp/src/linux/io_util_linux.cpp)` inside an `if(UNIX AND NOT APPLE)` guard. If everything is in headers, no action needed. |
| Linker errors about undefined functions from `io_util_linux.cpp` | Same as above — `.cpp` file not compiled | Add the `.cpp` as a source to the target |
| `SIOCGIFHWADDR` / `<linux/sockios.h>` errors | Missing Linux kernel headers | `sudo apt install -y linux-libc-dev` (usually already installed) |

#### 0b.8 — Run Tests

```bash
cd ~/pensar/build
ctest --output-on-failure -j$(nproc)
```

**Expected issues:**
- Tests that create files in hardcoded macOS paths will fail — fix to use portable paths.
- If tests reference `W("C:\\out\\log.txt")` or similar Windows paths, they'll harmlessly create files in unexpected locations. Check log_test and io_util_test output.

#### 0b.9 — Also Build contact_tests

```bash
cmake --build build --target contact_tests -j$(nproc)
# If it builds successfully:
cd build && ctest --output-on-failure -R contact
```

#### 0b.10 — Record Results

Document every fix. Commit with clear messages like `fix: Linux — remove erroneous Windows includes from io_util_linux.cpp` or `fix: Linux — use <filesystem> instead of <experimental/filesystem>`.

---

### Step 0c. macOS (Current Machine) — Verify Baseline

```bash
cd ~/pensar
cmake -S . -B build
cmake --build build --target pensar_tests
cd build && ctest --output-on-failure
```

This should already work — it's your current development environment. The goal is just to confirm nothing was broken by the CMakeLists.txt edits from Steps 0a/0b.

---

### Step 0d. (Recommended) Set Up GitHub Actions CI

Create `.github/workflows/ci.yml` so every future step is automatically verified on all three platforms:

```yaml
name: CI
on: [push, pull_request]
jobs:
  build:
    strategy:
      matrix:
        os: [macos-latest, ubuntu-latest, windows-latest]
    runs-on: ${{ matrix.os }}
    steps:
      - uses: actions/checkout@v4

      - name: Install ICU (Ubuntu)
        if: runner.os == 'Linux'
        run: sudo apt-get install -y libicu-dev

      - name: Install ICU (macOS)
        if: runner.os == 'macOS'
        run: |
          pip3 install conan
          conan profile detect
          conan install --requires="icu/76.1" --build=missing

      - name: Install ICU (Windows)
        if: runner.os == 'Windows'
        run: |
          pip install conan
          conan profile detect
          conan install --requires="icu/76.1" --build=missing

      - name: Configure
        run: cmake -S . -B build

      - name: Build
        run: cmake --build build --target pensar_tests

      - name: Test
        run: cd build && ctest --output-on-failure
```

(This is a starting point — you'll need to pass `ICU_ROOT` correctly for each platform. The CI will tell you exactly what breaks.)

---

### Prerequisites (after Step 0 is green)

**P0. Install Clang 18+ on macOS M4**
- `brew install llvm` installs Clang 18/19 to `/opt/homebrew/opt/llvm/`. Already works on Apple Silicon.
- Set `CC` / `CXX` to the Homebrew clang, or pass `-DCMAKE_CXX_COMPILER=/opt/homebrew/opt/llvm/bin/clang++` to CMake.
- Apple Clang (Xcode) does NOT have production-grade module support — Homebrew LLVM Clang is required.
- On Ubuntu: `apt install clang-18 libc++-18-dev`.

**P1. Bump CMake minimum to 3.28**
- In `CMakeLists.txt`, change `cmake_minimum_required(VERSION 3.20)` → `3.28`.
- Add `set(CMAKE_CXX_SCAN_FOR_MODULES ON)` after the project declaration.
- This enables first-class `FILE_SET CXX_MODULES` support needed for `target_sources`.

**P2. Verify green baseline with new toolchain**
- Build and run all tests with Clang 18+ (macOS/Linux) and MSVC (Windows) after the CMake bump. Fix any new warnings/errors before any module changes.

---

### Phase 1: Macro Elimination (Steps 1–4)

Modules cannot export `#define` macros. All cross-file macros must be eliminated or isolated *before* modularizing.

**Step 1. Replace `W()` macro with constexpr identity**
- In `cpp/src/string_def.hpp`, since `WIDE_CHAR` is commented out and `C = char`, `W(x)` is currently a no-op (`#define W(x) x`).
- Remove the `#define W(x) x` line.
- Add a `constexpr` identity: `constexpr const char* W(const char* x) { return x; }` and `constexpr char W(char x) { return x; }` — these are drop-in replacements since `W("literal")` and `W('c')` calls work identically.
- If wide-char support is needed in the future, `W()` can be made a `consteval` function or the `C` type alias switches, but no conditionality for now.
- **Test**: Build all tests on all three platforms — every file using `W()` must compile and pass unchanged.

**Step 2. Replace `LOG()` macro with inline function**
- In `cpp/src/log.hpp`, replace `#define LOG(msg)` and `#define LOG_FLUSH` with:
  - `inline void log_msg(const auto& msg, int line = 0)` that writes to `log_stream` if `log_on`.
  - `inline void log_flush()` that flushes `log_stream`.
- Update all ~12 call sites across the library (`object.hpp`, `file.hpp`, `icu_util.hpp`, `error.hpp`, `generator.hpp`, `command.hpp`, `io_util.hpp`, etc.) from `LOG(msg)` to `log_msg(msg)` (or use `std::source_location` to preserve line info).
- Remove `#define cpplog`, `#define LOG_ON`, `#define SHOW_LOCAL`, `#define CODEGEAR_BUG`.
- **Test**: Run `log_test.cpp` and all tests on all three platforms.

**Step 3. Replace `INCLUDE()` macro with CMake platform dispatch**
- The `INCLUDE(base_name)` macro in `cpp/src/multiplatform.hpp` generates `#include "macos/io_util_macos.hpp"` at preprocessing time. This is incompatible with `import`.
- **New approach**: Platform-specific files become **module partitions** selected by CMake:
  - In CMakeLists.txt, conditionally add the correct platform `.cppm` file: e.g. on macOS add `cpp/src/macos/io_util_macos.cppm`, on Windows add `cpp/src/windows/io_util_windows.cppm`, etc.
  - Each platform partition implements the same interface (e.g., `module cpplib:io_util_platform;`).
  - The main `cpplib:io_util` partition imports `cpplib:io_util_platform`.
- **Affected headers** (only 3): `io_util.hpp`, `system.hpp`, `sys_user_info.hpp`.
- For this step, keep them as headers but replace `#include INCLUDE(x)` with an explicit `#include "macos/x_macos.hpp"` guarded by `#ifdef __APPLE__` / `#ifdef _WIN32` / `#else` (Linux). This removes the `INCLUDE()` macro dependency while keeping `#include` temporarily.
- **Test**: Build on all three platforms, ensure platform-specific code works.

**Step 4. Isolate `data.hpp` verification macros**
- The ~30 macros in `cpp/src/data.hpp` (`VERIFY_DATA_STRUCT`, `ASSERT_NO_INTERNAL_PADDING`, etc.) are used in `static_assert` expressions in struct definitions across the library.
- **Strategy**: Keep these macros in a separate, non-module header: `cpp/src/data_macros.hpp`. This file will never become a module — consumers `#include "data_macros.hpp"` alongside `import cpplib;`.
- Move all macro definitions (lines ~100–368) from `data.hpp` into `data_macros.hpp`.
- `data.hpp` retains `TailPad<N>`, `StructLayoutDumper`, and `#include "data_macros.hpp"` at the bottom (for backward compat during transition).
- **Test**: Build all tests on all three platforms — struct verification `static_assert`s must still fire.

**Step 4b. Replace `FILE_LINE` macro in `macros.hpp`**
- Replace with `std::source_location::current()` usage or a constexpr helper.

---

### Phase 2: Fix Implicit Dependencies (Steps 5–6)

**Step 5. Add missing `#include`s to `class_info.hpp`**
- `cpp/src/class_info.hpp` uses `CS<>`, `VersionInt`, `S`, `SStream`, `equal<>`, `W()`, `std::span`, `EMPTY` — all without any `#include`. It relies on being textually included after `object.hpp`.
- Add explicit includes: `"constant.hpp"`, `"string_def.hpp"`, `"cs.hpp"`, `"equal.hpp"`, `<span>`.
- **Test**: Compile `class_info.hpp` standalone (add a trivial `.cpp` that includes only `class_info.hpp`). Build on all three platforms.

**Step 6. Break `cs.hpp` ↔ `icu_util.hpp` forward declaration**
- `cpp/src/cs.hpp` forward-declares `icu_util::remove_accent_char<CharT>()` to break a circular dependency. Forward declarations across module boundaries are not viable.
- **Refactor**: Extract the accent-related free functions (`remove_accent()`, `copy_remove_accent()`, and the char-level `equal()`/`less()` that use them) from `cs.hpp` into a new file `cpp/src/char_util.hpp`. This file includes `icu_util.hpp` directly — no forward declaration needed.
- `cs.hpp` no longer needs `icu_util.hpp` at all — `CS<>` delegates accent handling to `char_util.hpp`.
- Eliminate the redundant type alias re-declarations in `cs.hpp` (they're already in `string_def.hpp`).
- **Test**: Build all tests on all three platforms. Ensure accent-insensitive comparisons still work.

**Step 6b. Remove file-scope `namespace pd = ...` aliases**
- The alias `namespace pd = pensar_digital::cpplib;` in `cpp/src/constant.hpp` and ~5 other files won't propagate across module boundaries.
- Move all `pd` aliases to a dedicated non-module header `cpp/src/pd.hpp` that consumers `#include` alongside `import cpplib;`.
- **Test**: Build all tests on all three platforms.

---

### Phase 3: Module Conversion, Bottom-Up (Steps 7–12)

Each step creates `.cppm` module interface files, updates CMakeLists.txt with `target_sources(... FILE_SET CXX_MODULES ...)`, and converts the corresponding tests from `#include` to `import`.

**Step 7. Tier 0 — Leaf modules (no internal deps)**

Convert: `string_def`, `endian`, `binary_buffer`, `byte_order`, `random_util`, `span_util`, `statistic`, `mac_address`, `factory`.

- Create primary module interface `cpp/src/cpplib.cppm` that re-exports all partitions.
- **Test**: Convert `byte_order_test.cpp`, `random_util_test.cpp` to `import cpplib;`. Verify on all three platforms.

**Step 8. Tier 1–2 — Core types**

Convert: `constant`, `concept`, `log`, `memory`.

- **Test**: Convert `concept_test.cpp`, `log_test.cpp`. Verify on all three platforms.

**Step 9. Tier 3 — Bool, CS, Equal, Data**

Convert: `bool`, `cs`, `equal`, `data`, `clone_util`, `char_util`.

- `data_macros.hpp` stays as a non-module `#include` header.
- **Test**: Convert `code_util_test.cpp`, `binary_buffer_test.cpp`. Verify on all three platforms.

**Step 10. Tier 4–5 — Utilities and strings**

Convert: `code_util`, `encoding`, `array`, `wire_int`, `wire_double`, `icu_util`, `s`, `class_info`.

- ICU headers go in the global module fragment.
- **Test**: Convert `s_test.cpp`, `array-test.cpp`, `wire_double_test.cpp`, `distance_test.cpp`. Verify on all three platforms.

**Step 11. Tier 6–7 — Object and dependents**

Convert: `object`, `error`, `generator`, `constraint`, `stop_watch`, `command`.

- **Test**: Convert `object_test.cpp`, `factory_test.cpp`, `generator_test.cpp`, `command_test.cpp`, `constraint_test.cpp`, `stop_watch_test.cpp`. Verify on all three platforms.

**Step 12. Tier 8–9 — Platform-specific and high-level**

Convert: `system`, `sys_user_info`, `io_util`, `path`, `country`, `language`, `locale`, `file`, `sysinfo`, `sorted_list`.

- Platform dispatch: CMake `if(APPLE)` / `if(WIN32)` / `if(UNIX)` adds the correct platform partition `.cppm`.
- **Test**: Convert `io_util_test.cpp`, `file_test.cpp`, `path_test.cpp`, `sorted_list_test.cpp`, `algorithm_util_test.cpp`, `distance_test.cpp`. Finalize the umbrella `cpplib.cppm`. Verify on all three platforms.

---

### Phase 4: Cleanup (Step 13)

**Step 13. Remove old headers and finalize**
- Delete all `.hpp` files fully converted to `.cppm` (keep `data_macros.hpp` and `pd.hpp`).
- Update `copilot-instructions.md` to document module-based usage.
- Update `contact/` headers to use `import cpplib;`.
- Final full test run on all three platforms.

---

### Verification

After **every step**:
1. Build and test on macOS (Clang 18+ via Homebrew).
2. Build and test on Linux Ubuntu Server (Clang 18+ or GCC 14+).
3. Build and test on Windows (MSVC latest).
4. All existing tests must pass. No regressions.

Ideally use a GitHub Actions CI matrix (`macos-latest`, `ubuntu-latest`, `windows-latest`) set up in Step 0d.

### Decisions

- **Step 0 is the first step** — no code changes until macOS, Windows, and Linux all build and pass tests.
- **Known blockers on Linux**: `cpp/src/linux/io_util_linux.cpp` includes Windows headers — must be cleaned up. `cpp/src/linux/io_util_linux.hpp` uses `<experimental/filesystem>` — must be updated to `<filesystem>`.
- **Known blockers on Windows**: CMakeLists.txt ICU auto-detect uses Unix `/usr/bin/find` — must be replaced with CMake-native `file(GLOB_RECURSE ...)`. May need explicit link libraries (`ws2_32`, `iphlpapi`, `shell32`).
- **Compiler for Step 0**: Use whatever is native to each platform (Apple Clang on macOS, MSVC on Windows, GCC on Ubuntu). Clang 18+ comes later in the prerequisites phase.
- **`contact_db_tests` skipped** on Windows/Linux unless SOCI+PostgreSQL are installed — this is acceptable for now.
- **Clang 18+ via Homebrew** on macOS instead of Apple Clang (Apple Clang lacks production module support).
- **`W()` → constexpr function** since wide-char mode is disabled — mechanical, zero-risk replacement.
- **`data.hpp` macros stay as `#include`-only header** (`data_macros.hpp`) — too many macros to rewrite as constexpr templates, and they're compile-time-only tools.
- **Platform dispatch via CMake conditional `target_sources`** replaces the `INCLUDE()` macro.
- **No dual-mode coexistence** — each step fully cuts over from `#include` to `import`.
- **`namespace pd` alias** moves to a separate `#include`-only header.
