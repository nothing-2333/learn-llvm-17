cmake_minimum_required(VERSION 3.20.0)

# 设置构建类型为 Debug
set(CMAKE_BUILD_TYPE Debug)

# 移除 -fno-lifetime-dse 标志
string(REPLACE "-fno-lifetime-dse" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")

# 检查当前源代码目录是否是顶级源代码目录
# 如果是，则设置项目并进行初始化配置
if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
  # 定义项目名称为 Tinylang
  project(Tinylang)

  # 查找 LLVM 包，要求必须找到，并提供查找提示路径
  find_package(LLVM REQUIRED HINTS "${LLVM_CMAKE_PATH}")
  # 将 LLVM 的 CMake 模块路径添加到模块路径列表中
  list(APPEND CMAKE_MODULE_PATH ${LLVM_DIR})

  # 包含 LLVM 的自定义模块，用于添加 LLVM 相关目标
  include(AddLLVM)
  # 包含处理 LLVM 选项的模块
  include(HandleLLVMOptions)

  # 将 LLVM 的头文件目录添加到包含目录中
  include_directories("${LLVM_BINARY_DIR}/include" "${LLVM_INCLUDE_DIR}")
  # 将 LLVM 的库目录添加到链接目录中
  link_directories("${LLVM_LIBRARY_DIR}")

  # 设置标志，表示 Tinylang 是独立构建的
  set(TINYLANG_BUILT_STANDALONE 1)
endif()

# 将当前源代码目录下的 cmake/modules 文件夹添加到模块路径中
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules")

# 检查是否是原地构建（源代码目录和构建目录相同）
# 如果是，并且不是在 MSVC IDE 中，则报错
if(CMAKE_SOURCE_DIR STREQUAL CMAKE_BINARY_DIR AND NOT MSVC_IDE)
  message(FATAL_ERROR "In-source builds are not allowed. ")
endif()

# 设置 Tinylang 的版本字符串
set(TINYLANG_VERSION_STRING "0.1")
# 配置版本文件，从源代码目录中的模板文件生成目标文件
configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/include/tinylang/Basic/Version.inc.in
  ${CMAKE_CURRENT_BINARY_DIR}/include/tinylang/Basic/Version.inc)

# 包含自定义的 Tinylang 模块
include(AddTinylang)

# 将当前构建目录和源代码目录下的 include 文件夹添加到包含目录中
# 并且优先使用构建目录中的头文件
include_directories(BEFORE
  ${CMAKE_CURRENT_BINARY_DIR}/include
  ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# 添加 lib 子目录到构建系统中
add_subdirectory(lib)
# 添加 tools 子目录到构建系统中
add_subdirectory(tools)