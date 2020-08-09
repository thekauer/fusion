cmake_minimum_required(VERSION 3.14)
include(FetchContent)

find_package(LLVM 10 REQUIRED CONFIG)
SET (LLVM_DIR llvm/cmake/modules)
include_directories(${LLVM_INCLUDE_DIRS})
add_definitions(${LLVM_DEFINITIONS})


include_directories(../include)
file(GLOB FS_SRC ../src/*.cpp)
file(GLOB MAIN ../src/main.cpp)
list (REMOVE_ITEM FS_SRC ${MAIN})
file(GLOB srcs *.cpp)
set(CMAKE_CXX_STANDARD 17)
add_executable(test ${srcs} ${FS_SRC})
llvm_map_components_to_libnames(LLVM_LIBS
all
)
FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG        release-1.10.0
)
FetchContent_MakeAvailable(googletest)

set(CMAKE_CXX_FLAGS "-pthread")
target_link_libraries(test gtest gtest_main LLVM)

add_test(NAME test COMMAND test)
