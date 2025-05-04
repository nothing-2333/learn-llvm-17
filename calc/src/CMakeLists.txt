# 添加一个可执行文件目标，名为 "calc"
add_executable (calc
  Calc.cpp       # 添加 Calc.cpp 源文件到目标
  CodeGen.cpp    # 添加 CodeGen.cpp 源文件到目标
  Lexer.cpp      # 添加 Lexer.cpp 源文件到目标
  Parser.cpp     # 添加 Parser.cpp 源文件到目标
  Sema.cpp       # 添加 Sema.cpp 源文件到目标
)

# 为 "calc" 目标链接所需的 LLVM 库, PRIVATE 表示这些库只在 "calc" 目标内部使用，不会传递给依赖 "calc" 的其他目标
target_link_libraries(calc PRIVATE ${llvm_libs})

# 保留调试符号
target_compile_options(calc PRIVATE -g)