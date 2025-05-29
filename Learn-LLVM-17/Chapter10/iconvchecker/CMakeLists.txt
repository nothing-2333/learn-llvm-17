cmake_minimum_required(VERSION 3.20.0)
project(iconvchecker)

# 查找系统中安装的 LLVM 包，并要求必须找到它
find_package(LLVM REQUIRED CONFIG)

message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
# 输出 LLVMConfig.cmake 文件所在的路径
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

list(APPEND CMAKE_MODULE_PATH ${LLVM_DIR})
include(AddLLVM)
include(HandleLLVMOptions)

find_package(Clang REQUIRED)
list(APPEND CMAKE_MODULE_PATH ${Clang_DIR})
include(AddClang)

include_directories("${LLVM_INCLUDE_DIR}" "${CLANG_INCLUDE_DIRS}")
add_definitions("${LLVM_DEFINITIONS}")

link_directories("${LLVM_LIBRARY_DIR}")

# 添加一个 LLVM 模块库，名为 IconvChecker，源文件为 IconvChecker.cpp，
# 并指定它是一个插件工具，用于 clang
add_llvm_library(IconvChecker MODULE IconvChecker.cpp PLUGIN_TOOL clang)

# 如果是在 Windows 或 Cygwin 环境下
if(WIN32 OR CYGWIN)
  set(LLVM_LINK_COMPONENTS
    Support
    )
  clang_target_link_libraries(IconvChecker PRIVATE
    clangAnalysis
    clangAST
    clangStaticAnalyzerCore
    clangStaticAnalyzerFrontend
  )
endif()
