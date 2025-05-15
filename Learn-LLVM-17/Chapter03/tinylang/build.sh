cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DLLVM_DIR=/usr/local/lib/cmake/llvm -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++ -S . -B ./build
mv ./build/compile_commands.json .

cmake --build build/
mv ./build/tools/driver/tinylang .