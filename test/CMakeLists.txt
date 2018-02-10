set (
        TEST_SOURCE_FILES

        keva_test_main.cpp
        bp_node_test.cpp
)

include(External_GTest.cmake)
enable_testing()
include_directories(${GTEST_INCLUDE_DIRS})
include_directories(${GMOCK_INCLUDE_DIRS})

include_directories(${CMAKE_SOURCE_DIR}/src)

add_executable(keva-test ${TEST_SOURCE_FILES})
target_link_libraries(keva-test keva-lite gtest)
