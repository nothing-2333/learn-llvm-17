# 设置 CMake 的最低版本要求为 3.20.0。如果当前运行的 CMake 版本低于此版本，CMake 将停止处理项目并报告错误[^8^]。
cmake_minimum_required(VERSION 3.20.0)

# 定义项目名称为 "calc"，这将设置项目的基本信息，并允许后续命令使用项目名称[^14^]。
project("calc")

# 添加编译选项 -fno-rtti，该选项用于禁用 C++ 的运行时类型信息（RTTI），可以减少生成的二进制文件大小并提高性能[^11^]。
add_compile_options(-fno-rtti) 

# 加载 LLVM 包，将 LLVM 提供的 CMake 模块目录添加到搜索路径中。这使得后续可以使用 LLVM 提供的 CMake 函数和变量[^13^]。
find_package(LLVM REQUIRED CONFIG)

# 输出找到的 LLVM 版本和构建类型信息，方便开发者了解当前使用的 LLVM 版本[^10^]。
message("Found LLVM ${LLVM_PACKAGE_VERSION}, build type ${LLVM_BUILD_TYPE}")

# 将 LLVM 的 CMake 模块路径添加到 CMAKE_MODULE_PATH 中，以便后续可以使用 LLVM 提供的 CMake 模块[^13^]。
list(APPEND CMAKE_MODULE_PATH ${LLVM_DIR})

# 将 LLVM 的编译定义字符串拆分成一个列表，并存储到 LLVM_DEFINITIONS_LIST 变量中[^13^]。
separate_arguments(LLVM_DEFINITIONS_LIST NATIVE_COMMAND ${LLVM_DEFINITIONS})

# 将 LLVM 的编译定义添加到项目的编译选项中，确保在编译时这些定义会被传递给编译器[^13^]。
add_definitions(${LLVM_DEFINITIONS_LIST})

# 将 LLVM 的头文件目录添加到项目的头文件搜索路径中，并标记为系统头文件目录[^13^]。
include_directories(SYSTEM ${LLVM_INCLUDE_DIRS})

# 将 LLVM 的组件映射到对应的库名称。这里指定了需要使用 LLVM 的 Core 组件[^13^]。
llvm_map_components_to_libnames(llvm_libs Core)

# 添加源文件目录 "src" 作为子目录，以便在构建过程中处理该目录下的源文件[^11^]。
add_subdirectory("src")