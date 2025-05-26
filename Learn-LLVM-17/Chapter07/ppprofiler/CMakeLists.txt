cmake_minimum_required(VERSION 3.20.0)
project(ppprofilerpass)

# 查找LLVM包，要求必须找到（REQUIRED），并且使用其配置文件（CONFIG）
find_package(LLVM REQUIRED CONFIG)

message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

# 将LLVM的配置路径添加到CMake模块路径中，以便后续可以使用LLVM提供的CMake模块
list(APPEND CMAKE_MODULE_PATH ${LLVM_DIR})
# 包含LLVM提供的用于选择MSVC运行时库的CMake模块
include(ChooseMSVCCRT)

# 包含LLVM提供的用于添加LLVM Pass的CMake模块
include(AddLLVM)
# 包含LLVM提供的用于处理LLVM选项的CMake模块
include(HandleLLVMOptions)

# 将LLVM的头文件目录添加到包含目录中
include_directories("${LLVM_INCLUDE_DIR}")
# 添加LLVM的定义
add_definitions("${LLVM_DEFINITIONS}")

# 将LLVM的库目录添加到链接目录中
link_directories("${LLVM_LIBRARY_DIR}")

# 使用LLVM提供的add_llvm_pass_plugin函数添加LLVM Pass插件
# 参数说明：
# - PPProfiler：插件的目标名称
# - MODULE：指定这是一个模块
# - PPProfiler.cpp：插件的源代码文件
add_llvm_pass_plugin(PPProfiler MODULE PPProfiler.cpp)
