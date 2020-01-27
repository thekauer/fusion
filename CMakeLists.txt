cmake_minimum_required(VERSION 3.10)

# set the project name
project(fs)


find_package(LLVM REQUIRED CONFIG)
SET (LLVM_DIR llvm/cmake/modules) 
message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

file(GLOB fs_SRC
"*.h"
"*.cpp"
)
include_directories(${LLVM_INCLUDE_DIRS})
add_definitions(${LLVM_DEFINITIONS})


# add the executable
add_executable(fs  ${fs_SRC})


llvm_map_components_to_libnames(llvm_libs
 all
  )

target_link_libraries(fs ${LLVM})

set(CMAKE_CXX_STANDARD 20)
#dont show the buggy header guard
set(CMAKE_CXX_FLAGS "-std=c++2a -Wall -Wno-extra-tokens -Wno-char-subscripts -ferror-limit=3")
