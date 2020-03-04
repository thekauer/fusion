cmake_minimum_required(VERSION 3.10)

# set the project name
project(fs)


find_package(LLVM REQUIRED CONFIG)
SET (LLVM_DIR llvm/cmake/modules)
message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

message(STATUS "llvm include dirs: " ${LLVM_INCLUDE_DIRS})
#add_subdirectory(lld)
#add_subdirectory(googletest)
add_subdirectory(test)
include_directories(${LLVM_INCLUDE_DIRS})
add_definitions(${LLVM_DEFINITIONS})


include_directories(fs PRIVATE include)
file(GLOB FS_SRC "src/*cpp")
link_directories(include)
# add the executable
add_executable(fs  ${FS_SRC})
message("system: ${CMAKE_SYSTEM}")

llvm_map_components_to_libnames(LLVM_LIBS 
all
)
if(CMAKE_SYSTEM MATCHES "-arch")
target_link_libraries(fs LLVM)
else()
target_link_libraries(fs ${LLVM_LIBS})
endif()


set(CMAKE_CXX_STANDARD 20)
#dont show the buggy header guard
set(CMAKE_CXX_FLAGS "-std=c++2a -Wall -Wno-extra-tokens -Wno-char-subscripts -ferror-limit=3")
