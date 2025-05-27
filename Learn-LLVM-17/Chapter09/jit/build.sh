rm -rf build compile_commands.json JIT

cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
      -DLLVM_DIR=/usr/local/lib/cmake/llvm \
      -DCMAKE_C_COMPILER=clang \
      -DCMAKE_CXX_COMPILER=clang++ \
      -DCMAKE_BUILD_TYPE=Debug \
      -DCMAKE_EXE_LINKER_FLAGS="-fuse-ld=lld" -S . -B ./build 
      
ln -s ./build/compile_commands.json ./

cmake --build build/
ln -s ./build/JIT ./
