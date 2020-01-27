cmake_minimum_required(VERSION 3.10)

# set the project name
project(fs)

# SET (CMAKE_C_COMPILER             "/usr/bin/clang")
# SET (CMAKE_C_FLAGS                "-Wall -std=c99")
# SET (CMAKE_C_FLAGS_DEBUG          "-g")
# SET (CMAKE_C_FLAGS_MINSIZEREL     "-Os -DNDEBUG")
# SET (CMAKE_C_FLAGS_RELEASE        "-O4 -DNDEBUG")
# SET (CMAKE_C_FLAGS_RELWITHDEBINFO "-O2 -g")


# SET (CMAKE_CXX_COMPILER		    "/usr/bin/clang++")
# SET (CMAKE_CXX_FLAGS		    "-Wnoextra-tokens")
# SET (CMAKE_CXX_FLAGS_DEBUG	    "-g")
# SET (CMAKE_CXX_FLAGS_MINSIZEREL     "-Os -DNDEBUG")
# SET (CMAKE_CXX_FLAGS_RELEASE        "-O4 -DNDEBUG")
# SET (CMAKE_CXX_FLAGS_RELWITHDEBINFO "-O2 -g")


# SET (CMAKE_AR		"/usr/bin/llvm-ar")
# SET (CMAKE_LINKER	"/usr/bin/llvm-ld")
# SET (CMAKE_NM		"/usr/bin/llvm-nm")
# SET (CMAKE_OBJDUMP	"/usr/bin/llvm-objdump")
# SET (CMAKE_RANLIB	"/usr/bin/llvm-ranlib")



find_package(LLVM REQUIRED CONFIG)
SET (LLVM_DIR llvm/cmake/modules) 
message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

file(GLOB fs_SRC
"*.h"
"*.cpp"
)


# add the executable
add_executable(fs  ${fs_SRC})


llvm_map_components_to_libnames(llvm_libs
 all
  )

target_link_libraries(fs LLVM)

set(CMAKE_CXX_STANDARD 20)
#dont show the buggy header guard
set(CMAKE_CXX_FLAGS "-std=c++2a -Wall -Wno-extra-tokens -Wno-char-subscripts -ferror-limit=3")
