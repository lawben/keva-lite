cmake_minimum_required(VERSION 3.9)
project(keva_lite)

set(CMAKE_CXX_STANDARD 17)

set (
        SOURCE_FILES

        src/bp_node.cpp
        src/bp_node.hpp
        src/db_file_manager.cpp
        src/db_file_manager.hpp
        src/keva_lite.cpp
        src/keva_lite.hpp
        src/types.hpp
        src/utils.hpp
)

add_compile_options(-std=c++17 -Wall -Wextra -pedantic) # -Werror)

add_library(keva-lite STATIC ${SOURCE_FILES})

add_executable(keva-lite-example src/main.cpp)
target_link_libraries(keva-lite-example keva-lite)

add_subdirectory(test)

if (${CMAKE_BUILD_TYPE} STREQUAL "Debug")
    add_definitions(-DIS_DEBUG=1)
else()
    add_definitions(-DIS_DEBUG=0)
endif()