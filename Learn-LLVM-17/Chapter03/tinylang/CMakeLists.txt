cmake_minimum_required(VERSION 3.20.0)

# 检查当前源代码目录是否与顶级源代码目录相同
if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
  project(Tinylang)

  # 查找 LLVM 包，要求必须找到，并提供 LLVM CMake 路径作为提示
  find_package(LLVM REQUIRED HINTS "${LLVM_CMAKE_PATH}")
  # 将 LLVM 的 CMake 模块路径添加到 CMAKE_MODULE_PATH 中
  list(APPEND CMAKE_MODULE_PATH ${LLVM_DIR})

  # 包含 LLVM 的 CMake 模块
  include(AddLLVM)
  include(HandleLLVMOptions)

  # 包含 LLVM 的头文件目录
  include_directories("${LLVM_BINARY_DIR}/include" "${LLVM_INCLUDE_DIR}")
  # 添加 LLVM 的库目录到链接目录
  link_directories("${LLVM_LIBRARY_DIR}")

  # 设置 Tinylang 是否独立构建的标志
  set(TINYLANG_BUILT_STANDALONE 1)
endif()

# 将当前源代码目录中的 modules 子目录添加到 CMAKE_MODULE_PATH 中
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules")

# 检查是否在源代码目录中进行构建（非 MSVC IDE 环境）
if(CMAKE_SOURCE_DIR STREQUAL CMAKE_BINARY_DIR AND NOT MSVC_IDE)
  # 如果是，则报错，不允许在源代码目录中进行构建
  message(FATAL_ERROR "In-source builds are not allowed. ")
endif()

# 设置 Tinylang 的版本字符串
set(TINYLANG_VERSION_STRING "0.1")
configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/include/tinylang/Basic/Version.inc.in
  ${CMAKE_CURRENT_BINARY_DIR}/include/tinylang/Basic/Version.inc)

# 包含 Tinylang 的 CMake 模块
include(AddTinylang)

# 包含当前构建目录和源代码目录中的头文件目录
include_directories(BEFORE
  ${CMAKE_CURRENT_BINARY_DIR}/include
  ${CMAKE_CURRENT_SOURCE_DIR}/include
  )

  # 添加 lib 和 tools 子目录作为子项目
add_subdirectory(lib)
add_subdirectory(tools)
