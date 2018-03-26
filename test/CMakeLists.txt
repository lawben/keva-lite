set (
        TEST_SOURCE_FILES

        db_manager_test.cpp
        bp_node_test.cpp
        file_manager_test.cpp
        keva_test_main.cpp
        keva_lite_test.cpp
        test_utils.cpp
        test_utils.hpp
        utils_test.cpp
)

enable_testing()
include(External_GTest.cmake)

include_directories(${GTEST_INCLUDE_DIRS})
include_directories(${GMOCK_INCLUDE_DIRS})
include_directories(${CMAKE_SOURCE_DIR}/src)

add_executable(keva-test ${TEST_SOURCE_FILES})
target_link_libraries(keva-test keva-lite ${GTEST_LIBRARY})
