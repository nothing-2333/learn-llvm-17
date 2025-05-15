cmake_minimum_required (VERSION 3.20.0)

project ("calc")

# 查找 LLVM 包，要求必须找到，并且使用配置模式
find_package(LLVM REQUIRED CONFIG)
# 输出找到的 LLVM 版本和构建类型信息
message("Found LLVM ${LLVM_PACKAGE_VERSION}, build type ${LLVM_BUILD_TYPE}")
# 将 LLVM 的 CMake 模块路径添加到 CMAKE_MODULE_PATH 中
list(APPEND CMAKE_MODULE_PATH ${LLVM_DIR})
# 包含用于确定 GCC 兼容性的模块
include(DetermineGCCCompatible)
# 包含用于选择 MSVC CRT（C运行时库）的模块
include(ChooseMSVCCRT)

# 将 LLVM 定义的宏从字符串分割成列表
separate_arguments(LLVM_DEFINITIONS_LIST NATIVE_COMMAND ${LLVM_DEFINITIONS})
# 将 LLVM 的定义添加到编译器定义中
add_definitions(${LLVM_DEFINITIONS_LIST})
# 将 LLVM 的头文件目录添加到系统头文件目录中
include_directories(SYSTEM ${LLVM_INCLUDE_DIRS})
# 将 LLVM 的组件映射到库名称，这里只使用了 Core 组件
llvm_map_components_to_libnames(llvm_libs Core)

# 如果 LLVM 编译器是 GCC 兼容的
if(LLVM_COMPILER_IS_GCC_COMPATIBLE)
  # 如果 LLVM 没有启用 RTTI（运行时类型信息）
  if(NOT LLVM_ENABLE_RTTI)
    # 在 C++ 编译器标志中添加 -fno-rtti
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-rtti")
  endif()
  # 如果 LLVM 没有启用异常处理
  if(NOT LLVM_ENABLE_EH)
    # 在 C++ 编译器标志中添加 -fno-exceptions
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-exceptions")
  endif()
endif()

# 添加 src 目录作为子目录，以便构建其中的源文件
add_subdirectory ("src")
