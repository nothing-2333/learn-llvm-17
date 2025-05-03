cmake -GNinja -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++ -DLLVM_DIR=/usr/local/lib/cmake/llvm -S . -B ./build
cd build
ninja

./src/calc "with a: a*3" | llc -filetype=obj -relocation-model=pic -o expr.o
clang -o ../expr ./expr.o ../rtcalc.c 