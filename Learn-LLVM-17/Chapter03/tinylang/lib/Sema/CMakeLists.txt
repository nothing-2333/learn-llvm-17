# 设置 LLVM 链接组件
set(LLVM_LINK_COMPONENTS support)

# 使用了之前定义的 add_tinylang_library 宏来添加一个库
add_tinylang_library(tinylangSema
  Scope.cpp
  Sema.cpp

  LINK_LIBS
  tinylangBasic
  )
