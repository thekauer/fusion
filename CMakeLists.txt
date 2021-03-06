cmake_minimum_required(VERSION 3.10)

# set the project name
project(fs)

SET(CMAKE_FIND_PACKAGE_SORT_ORDER NATURAL)
SET(CMAKE_FIND_PACKAGE_SORT_DIRECTION DEC) #put latest llvm instalation first find latest
find_package(LLVM 10 REQUIRED CONFIG)
SET (LLVM_DIR llvm/cmake/modules)
message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

message(STATUS "llvm include dirs: " ${LLVM_INCLUDE_DIRS})
message("system: ${CMAKE_SYSTEM}")
#add_subdirectory(lld)
#add_subdirectory(googletest)
add_subdirectory(test)
include_directories(${LLVM_INCLUDE_DIRS})
add_definitions(${LLVM_DEFINITIONS})
link_directories(${LLVM_LIBRARY_DIRS})

include_directories(fs PRIVATE include)
file(GLOB FS_SRC "src/*cpp")
link_directories(include)
# add the executable
add_executable(fs  ${FS_SRC})

llvm_map_components_to_libnames(LLVM_LIBS
core support irreader
)

message(STATUS "llvm libs: " ${LLVM_LIBS})
target_link_libraries(fs LLVM)

set(CMAKE_CXX_STANDARD 17)
#dont show the buggy header guard
set(CMAKE_CXX_FLAGS "-std=c++17 -Wall -Wno-extra-tokens -Wno-char-subscripts -ferror-limit=3")
