cmake_minimum_required (VERSION 3.20.0)

# 设置 C++ 标准为 C++17，并且要求必须支持该标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project ("tablegen")

# 查找 LLVM，并要求必须找到 LLVM 的配置
find_package(LLVM REQUIRED CONFIG)
# 输出找到的 LLVM 版本和构建类型
message("Found LLVM ${LLVM_PACKAGE_VERSION}, build type ${LLVM_BUILD_TYPE}")
# 将 LLVM 的 CMake 模块路径添加到 CMAKE_MODULE_PATH
list(APPEND CMAKE_MODULE_PATH ${LLVM_DIR})

# 将 LLVM 的定义拆分为单独的参数，并添加到编译定义中
separate_arguments(LLVM_DEFINITIONS_LIST NATIVE_COMMAND ${LLVM_DEFINITIONS})
add_definitions(${LLVM_DEFINITIONS_LIST})
# 将 LLVM 的头文件目录添加到系统头文件目录中
include_directories(SYSTEM ${LLVM_INCLUDE_DIRS})
# 将支持和 TableGen 相关的 LLVM 组件映射到库名称
llvm_map_components_to_libnames(llvm_libs support tablegen)

# 包含用于确定 GCC 兼容性的模块
include(DetermineGCCCompatible)
if(MSVC)
  # 将构建类型转换为大写
  string(TOUPPER "${CMAKE_BUILD_TYPE}" build)
  # 根据 LLVM 的 CRT 设置，调整 MSVC 的运行时库选项
  string(REGEX REPLACE "^[-/]?M([DT])(d?)$"
         "\\2\\1" CMAKE_MSVC_RUNTIME_LIBRARY
         ${LLVM_USE_CRT_${build}})
  string(REPLACE "T"
         "" CMAKE_MSVC_RUNTIME_LIBRARY
         ${CMAKE_MSVC_RUNTIME_LIBRARY})
  string(REPLACE "D"
         "DLL" CMAKE_MSVC_RUNTIME_LIBRARY
         ${CMAKE_MSVC_RUNTIME_LIBRARY})
  string(REPLACE "d"
         "Debug" CMAKE_MSVC_RUNTIME_LIBRARY
         ${CMAKE_MSVC_RUNTIME_LIBRARY})
  set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded${CMAKE_MSVC_RUNTIME_LIBRARY}")
  message("CMAKE_MSVC_RUNTIME_LIBRARY: ${CMAKE_MSVC_RUNTIME_LIBRARY}")
endif()

# 如果启用了异常处理 (LLVM_ENABLE_EH)
if(LLVM_ENABLE_EH)
  if(MSVC)
    # 添加异常处理标志
    list(APPEND LLVM_COMPILE_FLAGS "/EHsc")
  endif()
else()
  # 如果编译器是 GCC 兼容的
  if(LLVM_COMPILER_IS_GCC_COMPATIBLE)
    # 禁用异常处理
    list(APPEND LLVM_COMPILE_FLAGS "-fno-exceptions")
    # 如果未启用 unwind 表
    if(NOT LLVM_ENABLE_UNWIND_TABLES)
      list(APPEND LLVM_COMPILE_FLAGS "-fno-unwind-tables")
      list(APPEND LLVM_COMPILE_FLAGS "-fno-asynchronous-unwind-tables")
    endif()
  elseif(MSVC)
    # 添加定义以禁用异常处理
    list(APPEND LLVM_COMPILE_DEFINITIONS _HAS_EXCEPTIONS=0)
    list(APPEND LLVM_COMPILE_FLAGS "/EHs-c-")
  endif()
endif()

# 如果未启用 RTTI (LLVM_ENABLE_RTTI)
if(NOT LLVM_ENABLE_RTTI)
  if (LLVM_COMPILER_IS_GCC_COMPATIBLE)
    # 禁用 RTTI
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-rtti")
  elseif (MSVC)
    list(APPEND LLVM_COMPILE_FLAGS "/GR-")
  endif()
elseif(MSVC)
  list(APPEND LLVM_COMPILE_FLAGS "/GR")
endif()

# 添加标志以使用本地架构优化
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=native")

add_executable (tinylang-tblgen
  TableGen.cpp
  TokenEmitter.cpp
  )
# 将 LLVM 的库链接到 tinylang-tblgen
target_link_libraries(tinylang-tblgen PRIVATE ${llvm_libs})
