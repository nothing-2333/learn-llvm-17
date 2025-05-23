rm -rf build compile_commands.json expr

cmake -GNinja -DCMAKE_BUILD_TYPE=Debug -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DLLVM_DIR=/usr/local/lib/cmake/llvm -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++ \
      -DCMAKE_EXE_LINKER_FLAGS="-fuse-ld=lld" -S . -B ./build
cd build
ninja
ln -s ./compile_commands.json ../

./src/calc "with a: 3/a" | llc -filetype=obj -relocation-model=pic -o expr.o
clang++ -o ../expr ./expr.o ../rtcalc.cpp
cd ..
