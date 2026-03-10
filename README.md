# pensar

Pensar Digital global git repository.
cpplib is a c++ library with basic utilities used by all Pensar Digital projects.



Based on the workspace tasks and CMake setup, here's how to build and run all tests:

```bash
cd /Users/mg/pensar

# 1. Configure (only needed once or after CMakeLists.txt changes)
cmake -S . -B build

# 2. Build all test targets
cmake --build build

# 3. Run all tests
cd build && ctest --output-on-failure -j$(sysctl -n hw.logicalcpu)
```

To build/run a specific test target:

```bash
# pensar_tests only
cmake --build build --target pensar_tests && ./build/pensar_tests

# contact_tests only
cmake --build build --target contact_tests && ./build/contact_tests --colour-mode ansi

# contact_db_tests only
cmake --build build --target contact_db_tests && ./build/contact_db_tests
```

The `-j$(sysctl -n hw.logicalcpu)` flag runs CTest in parallel using all available CPU cores on your Mac.