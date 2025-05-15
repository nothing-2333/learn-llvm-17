# 添加一个可执行目标（即生成一个可执行文件）
add_executable (calc
  Calc.cpp          # 添加 Calc.cpp 源文件
  CodeGen.cpp       # 添加 CodeGen.cpp 源文件
  Lexer.cpp         # 添加 Lexer.cpp 源文件
  Parser.cpp        # 添加 Parser.cpp 源文件
  Sema.cpp          # 添加 Sema.cpp 源文件
  )

# 为 calc 可执行目标链接所需的库
# PRIVATE 表示这些库只对 calc 可执行目标本身可见，不会传递给其他目标
target_link_libraries(calc PRIVATE ${llvm_libs})