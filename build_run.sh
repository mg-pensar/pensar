cmake --build build
cd build
ctest --output-on-failure -j$(sysctl -n hw.logicalcpu)
cd ..

