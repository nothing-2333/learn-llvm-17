rm -rf build compile_commands.json tinylang

cmake -DCMAKE_BUILD_TYPE=Debug -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DLLVM_DIR=/usr/local/lib/cmake/llvm \
      -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++ \
      -DCMAKE_CXX_FLAGS="-fpass-plugin=/usr/local/lib/PPProfiler.so" \
      -DCMAKE_EXE_LINKER_FLAGS="-fuse-ld=lld /home/nothing/learn-llvm-17/Learn-LLVM-17/Chapter07/tinylang/runtime.o" -S . -B ./build 
      
      
ln -s ./build/compile_commands.json ./

cmake --build build/
ln -s ./build/tools/driver/tinylang ./
