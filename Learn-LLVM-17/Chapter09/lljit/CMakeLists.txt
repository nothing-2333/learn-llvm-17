﻿cmake_minimum_required (VERSION 3.20.0)

project ("jit")

find_package(LLVM REQUIRED CONFIG)
list(APPEND CMAKE_MODULE_PATH ${LLVM_DIR})
include(DetermineGCCCompatible)
include(ChooseMSVCCRT)

add_definitions(${LLVM_DEFINITIONS})
include_directories(SYSTEM ${LLVM_INCLUDE_DIRS})
llvm_map_components_to_libnames(llvm_libs Core OrcJIT Support native)

if(LLVM_COMPILER_IS_GCC_COMPATIBLE)
  if(NOT LLVM_ENABLE_RTTI)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-rtti")
  endif()
  if(NOT LLVM_ENABLE_EH)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-exceptions")
  endif()
endif()

add_executable (calc
  Calc.cpp
  CodeGen.cpp
  Lexer.cpp
  Parser.cpp
  Sema.cpp
  )

# 添加符号导出指令
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
  target_link_options(calc PRIVATE
    "-Wl,--export-dynamic-symbol=llvm_orc_registerEHFrameSectionWrapper"
    "-Wl,--export-dynamic-symbol=llvm_orc_deregisterEHFrameSectionWrapper"
    "-Wl,--export-dynamic-symbol=llvm_orc_registerJITLoaderGDBWrapper"
  )
elseif(CMAKE_SYSTEM_NAME STREQUAL "Windows")
  # Windows平台需要不同的处理方式
  target_compile_definitions(calc PRIVATE "LLVM_ORC_REGISTER_EH_FRAME_SECTION_WRAPPER=1")
  set_target_properties(calc PROPERTIES LINK_FLAGS "/EXPORT:llvm_orc_registerEHFrameSectionWrapper")
endif()

target_link_libraries(calc PRIVATE ${llvm_libs})
