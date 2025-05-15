# 设置 LLVM 链接组件
set(LLVM_LINK_COMPONENTS
  Support # 包含 LLVM 的支持组件
  )

# 添加 Tinylang 的工具目标
add_tinylang_tool(tinylang
  Driver.cpp  # 指定工具的源文件
  )

# 为 tinylang 工具目标链接所需的库
target_link_libraries(tinylang
  PRIVATE # 指定链接的库仅对 tinylang 可见
  tinylangBasic # 链接 tinylangBasic 库
  tinylangLexer # 链接 tinylangLexer 库
  tinylangParser  # 链接 tinylangParser 库
  tinylangSema  # 链接 tinylangSema 库
  )
