# 定义一个名为 "calc" 的可执行文件。
# 这个命令告诉 CMake 将指定的源文件编译成一个可执行文件。
# 源文件列表包括 Calc.cpp、CodeGen.cpp、Lexer.cpp、Parser.cpp 和 Sema.cpp。
# 这些源文件应该位于当前目录或子目录中。
add_executable (calc
    Calc.cpp CodeGen.cpp Lexer.cpp Parser.cpp Sema.cpp)

# 为 "calc" 可执行文件指定链接的库。
# 这个命令告诉 CMake 在链接 "calc" 时需要链接哪些库。
# 这里使用了 PRIVATE 关键字，表示这些库只在 "calc" 可执行文件内部使用，不会被其他目标继承。
# ${llvm_libs} 是一个变量，包含了通过 llvm_map_components_to_libnames 函数映射得到的 LLVM 库列表。
# 这些库是构建 "calc" 时所需的 LLVM 组件库。
target_link_libraries(calc PRIVATE ${llvm_libs})

# 保留调试符号
target_compile_options(calc PRIVATE -g)