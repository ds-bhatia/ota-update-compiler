cmake_minimum_required(VERSION 3.13)
project(OTASecurityPass)

find_package(LLVM REQUIRED CONFIG)

message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

# The pass shared library (loaded by opt)
add_library(OTASecurityPass SHARED TraversalPass.cpp)

target_include_directories(OTASecurityPass PRIVATE ${LLVM_INCLUDE_DIRS})

set_target_properties(OTASecurityPass PROPERTIES
    COMPILE_FLAGS "-fno-rtti"
)

# Keep old name as symlink/alias for backward compat
set_target_properties(OTASecurityPass PROPERTIES
    OUTPUT_NAME "OTASecurityPass"
)

