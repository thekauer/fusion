cmake_minimum_required(VERSION 3.10)


find_package(LLVM REQUIRED CONFIG)
SET (LLVM_DIR llvm/cmake/modules)   
include_directories(${LLVM_INCLUDE_DIRS})
add_definitions(${LLVM_DEFINITIONS})



include_directories(../include)
file(GLOB FS_SRC ../src/*.cpp)
file(GLOB MAIN ../src/main.cpp)
list (REMOVE_ITEM FS_SRC ${MAIN})
file(GLOB srcs *.cpp)
set(CMAKE_CXX_STANDARD 20)
add_executable(test ${srcs} ${FS_SRC})
llvm_map_components_to_libnames(LLVM_LIBS 
all
)

if(CMAKE_SYSTEM MATCHES "-arch")
target_link_libraries(test gtest_main LLVM)
else()
target_link_libraries(test gtest_main LLVM})
endif()

add_test(NAME test COMMAND test)