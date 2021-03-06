# Taken from: https://gist.github.com/ClintLiddick/51deffb768a7319e715071aa7bd3a3ab
find_package(Threads REQUIRED)
include(ExternalProject)

ExternalProject_Add(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG cafa9e52285d743d1394900fa95662950f712a72
        UPDATE_COMMAND ""
        INSTALL_COMMAND ""
        BUILD_COMMAND make CXX_FLAGS=${GTEST_CXX_FLAGS} LDFLAGS=${GTEST_LINKER_FLAGS}
        #LOG_DOWNLOAD ON  # enable if there are problems with building googltest
        #LOG_CONFIGURE ON
        #LOG_BUILD ON
)

ExternalProject_Get_Property(googletest source_dir)
set(GTEST_INCLUDE_DIRS ${source_dir}/googletest/include)
set(GMOCK_INCLUDE_DIRS ${source_dir}/googlemock/include)

ExternalProject_Get_Property(googletest binary_dir)
set(GTEST_LIBRARY_PATH ${binary_dir}/googlemock/gtest/${CMAKE_FIND_LIBRARY_PREFIXES}gtest.a)
set(GTEST_LIBRARY gtest)
add_library(${GTEST_LIBRARY} UNKNOWN IMPORTED)
set_target_properties(${GTEST_LIBRARY} PROPERTIES
        IMPORTED_LOCATION ${GTEST_LIBRARY_PATH}
        IMPORTED_LINK_INTERFACE_LIBRARIES "${CMAKE_THREAD_LIBS_INIT}")
add_dependencies(${GTEST_LIBRARY} googletest)
