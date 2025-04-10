cmake_minimum_required(VERSION 3.10)
project(DFGPass)

set(LLVM_DIR /usr/lib/llvm-12/share/llvm/cmake)

find_package(LLVM REQUIRED CONFIG)

add_definitions(${LLVM_DEFINITIONS_LIST})
include_directories(${LLVM_INCLUDE_DIRS})

list(APPEND CMAKE_MODULE_PATH "${LLVM_CMAKE_DIR}")
include(AddLLVM)

add_subdirectory(DFGPass)
