cmake_minimum_required(VERSION 3.13)
project(TraversalPass)

find_package(LLVM REQUIRED CONFIG)

add_library(TraversalPass SHARED TraversalPass.cpp)

target_include_directories(TraversalPass PRIVATE ${LLVM_INCLUDE_DIRS})

set_target_properties(TraversalPass PROPERTIES
    COMPILE_FLAGS "-fno-rtti"
)

