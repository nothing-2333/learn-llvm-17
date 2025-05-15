# 定义一个宏，用于添加 Tinylang 的子目录
macro(add_tinylang_subdirectory name)
  # 调用 LLVM 提供的宏 add_llvm_subdirectory，将子目录添加到构建系统中
  # 参数 TINYLANG 表示项目名称，TOOL 表示项目类型，${name} 是子目录名称
  add_llvm_subdirectory(TINYLANG TOOL ${name})
endmacro()

# 定义一个宏，用于添加 Tinylang 的库
macro(add_tinylang_library name)
  # 根据 BUILD_SHARED_LIBS 的值决定生成共享库还是静态库
  if(BUILD_SHARED_LIBS)
    set(LIBTYPE SHARED) # 如果 BUILD_SHARED_LIBS 为真，生成共享库
  else()
    set(LIBTYPE STATIC) # 否则，生成静态库
  endif()
  # 调用 LLVM 提供的宏 llvm_add_library，添加库目标
  # ${name} 是库的名称，${LIBTYPE} 是库的类型，${ARGN} 是传递给宏的其他参数
  llvm_add_library(${name} ${LIBTYPE} ${ARGN})
  # 如果目标存在
  if(TARGET ${name})
    # 为库目标添加链接的接口库
    target_link_libraries(${name} INTERFACE ${LLVM_COMMON_LIBS})
    # 安装库目标
    install(TARGETS ${name}
      COMPONENT ${name}
      LIBRARY DESTINATION lib${LLVM_LIBDIR_SUFFIX}
      ARCHIVE DESTINATION lib${LLVM_LIBDIR_SUFFIX}
      RUNTIME DESTINATION bin)
  else()
    # 如果目标不存在，添加一个自定义目标
    add_custom_target(${name})
  endif()
endmacro()

# 定义一个宏，用于添加 Tinylang 的可执行文件
macro(add_tinylang_executable name)
  # 调用 LLVM 提供的宏 add_llvm_executable，添加可执行文件目标
  # ${name} 是可执行文件的名称，${ARGN} 是传递给宏的其他参数
  add_llvm_executable(${name} ${ARGN} )
endmacro()

# 定义一个宏，用于添加 Tinylang 的工具
macro(add_tinylang_tool name)
  # 调用 add_tinylang_executable 宏，添加可执行文件目标
  add_tinylang_executable(${name} ${ARGN})
  # 安装可执行文件目标
  install(TARGETS ${name}
    RUNTIME DESTINATION bin # 可执行文件安装路径
    COMPONENT ${name})  # 安装组件名称
endmacro()
