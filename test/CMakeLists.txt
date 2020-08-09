cmake_minimum_required(VERSION 3.10)

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

configure_file(CMakeLists.txt.in
        CMakeLists.txt)
execute_process(COMMAND ${CMAKE_COMMAND} -G "${CMAKE_GENERATOR}" .
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/test )
execute_process(COMMAND ${CMAKE_COMMAND} --build .
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/test )


set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)


add_subdirectory(${CMAKE_BINARY_DIR}/googletest-src
        ${CMAKE_BINARY_DIR}/googletest-build)


set(CMAKE_CXX_FLAGS "-pthread")
target_link_libraries(test gtest gtest_main LLVM)

add_test(NAME test COMMAND test)
