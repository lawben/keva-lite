cmake_minimum_required(VERSION 3.9)
project(keva_lite)

set(CMAKE_CXX_STANDARD 17)

set (
        SOURCE_FILES

        src/bp_node.cpp
        src/bp_node.hpp
        src/db_manager.cpp
        src/db_manager.hpp
        src/keva_lite.hpp
        src/types.hpp
        src/utils.hpp
        src/file_manager.cpp
        src/file_manager.hpp
)

set(CMAKE_CXX_FLAGS "-std=c++1z -Wall -Wextra -pedantic -Werror")

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -lc++ -lc++abi")
    # We need to make sure gtest is built just like the main project
    set(GTEST_CXX_FLAGS "${CXX_FLAGS} ${CMAKE_CXX_FLAGS}")
    set(GTEST_LINKER_FLAGS "${LDFLAGS} ${CMAKE_EXE_LINKER_FLAGS}")
endif()

# Set default build type if none was passed on the command line
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release")
endif()

if ("${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
    add_definitions(-DIS_DEBUG=1)
else()
    add_definitions(-DIS_DEBUG=0)
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -march=native")
endif()

# Set to build with ASAN and UBSAN enabled
if (DEFINED ENV{ENABLE_SANITIZE})
    set(SANITIZE_FLAGS "-fsanitize=address,undefined -fno-sanitize-recover=all -fno-omit-frame-pointer")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${SANITIZE_FLAGS}")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${SANITIZE_FLAGS}")
endif()


add_library(keva-lite STATIC ${SOURCE_FILES})
add_executable(keva-lite-example src/main.cpp)
target_link_libraries(keva-lite-example keva-lite)

add_subdirectory(test)
