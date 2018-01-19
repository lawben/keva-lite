cmake_minimum_required(VERSION 3.9)
project(keva_lite)

set(CMAKE_CXX_STANDARD 17)

set (
        SOURCE_FILES
        src/main.cpp
)

add_compile_options(-std=c++17 -Wall -Wextra -pedantic -Werror)

include_directories(src)

add_executable(keva-lite ${SOURCE_FILES})