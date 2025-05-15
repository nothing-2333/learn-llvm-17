cmake -GNinja -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DLLVM_DIR=/usr/local/lib/cmake/llvm -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++ -S . -B ./build
cd build
ninja
mv ./compile_commands.json ..

./src/calc "with a: a*3" | llc -filetype=obj -relocation-model=pic -o expr.o
clang -o ../expr ./expr.o ../rtcalc.c
cd ..
