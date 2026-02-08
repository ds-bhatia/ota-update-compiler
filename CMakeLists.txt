cmake_minimum_required(VERSION 3.13)
project(TraversalPass)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(LLVM 14 REQUIRED CONFIG)

include_directories(${LLVM_INCLUDE_DIRS})
add_definitions(${LLVM_DEFINITIONS})

add_library(TraversalPass SHARED pass/TraversalPass.cpp)

# IMPORTANT:
# Do NOT link against LLVM libraries for a plugin
# opt/clang already provide LLVM at runtime

