# 定义一个宏，用于添加 TinyLang 的子目录
macro(add_tinylang_subdirectory name)
  # 调用 LLVM 的 add_llvm_subdirectory 宏，将指定的子目录作为工具（TOOL）添加到 TinyLang 项目中
  add_llvm_subdirectory(TINYLANG TOOL ${name})
endmacro()

# 定义一个宏，用于添加 TinyLang 的库
macro(add_tinylang_library name)
  # 检查是否构建共享库
  if(BUILD_SHARED_LIBS)
    # 如果是，设置库类型为 SHARED（共享库）
    set(LIBTYPE SHARED)
  else()
    # 如果不是，设置库类型为 STATIC（静态库）
    set(LIBTYPE STATIC)
  endif()
  # 调用 LLVM 的 llvm_add_library 宏，添加指定名称的库，并根据 LIBTYPE 参数决定是静态库还是共享库
  # ${ARGN} 表示传递给宏的额外参数，通常包括源文件等
  llvm_add_library(${name} ${LIBTYPE} ${ARGN})
  # 检查是否成功创建了目标
  if(TARGET ${name})
    # 如果是，为目标添加接口链接库，链接 LLVM 的通用库
    target_link_libraries(${name} INTERFACE ${LLVM_COMMON_LIBS})
    # 安装目标，指定安装路径和组件
    install(TARGETS ${name}
      COMPONENT ${name}
      LIBRARY DESTINATION lib${LLVM_LIBDIR_SUFFIX}  # 库文件安装到 lib${LLVM_LIBDIR_SUFFIX} 目录
      ARCHIVE DESTINATION lib${LLVM_LIBDIR_SUFFIX}  # 归档文件安装到 lib${LLVM_LIBDIR_SUFFIX} 目录
      RUNTIME DESTINATION bin)                      # 可执行文件安装到 bin 目录
  else()
    # 如果没有成功创建目标，添加一个自定义目标
    add_custom_target(${name})
  endif()
endmacro()

# 定义一个宏，用于添加 TinyLang 的可执行文件
macro(add_tinylang_executable name)
  # 调用 LLVM 的 add_llvm_executable 宏，添加指定名称的可执行文件
  # ${ARGN} 表示传递给宏的额外参数，通常包括源文件等
  add_llvm_executable(${name} ${ARGN} )
endmacro()

# 定义一个宏，用于添加 TinyLang 的工具
macro(add_tinylang_tool name)
  # 调用 add_tinylang_executable 宏，添加可执行文件
  add_tinylang_executable(${name} ${ARGN})
  # 安装目标，指定安装路径和组件
  install(TARGETS ${name}
    RUNTIME DESTINATION bin  # 可执行文件安装到 bin 目录
    COMPONENT ${name})      # 指定安装组件为 ${name}
endmacro()