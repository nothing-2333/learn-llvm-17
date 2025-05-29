rm -rf build compile_commands.json

cmake -GNinja -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
      -DCMAKE_C_COMPILER=clang \
      -DCMAKE_CXX_COMPILER=clang++ \
      -DLLVM_DIR=/usr/local/lib/cmake/llvm -DClang_DIR=/usr/local/lib/cmake/clang/ \
      -S . -B ./build 
      
ln -s ./build/compile_commands.json ./

cmake --build build/
