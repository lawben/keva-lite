cmake_minimum_required(VERSION 3.9)
project(keva_lite)
set(NAMESPACE keva)

set(CMAKE_CXX_STANDARD 17)

set (
        SOURCE_FILES
        src/main.cpp
        src/keva_lite.cpp
        src/keva_lite.hpp
        src/bp_node.cpp src/bp_node.hpp src/db_file_manager.cpp src/db_file_manager.hpp src/utils.hpp)

add_compile_options(-std=c++17 -Wall -Wextra -pedantic) # -Werror)

include_directories(src)

add_executable(keva-lite ${SOURCE_FILES})