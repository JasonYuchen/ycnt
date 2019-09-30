cmake_minimum_required(VERSION 3.10)
project(ycnt)

set(CMAKE_CXX_STANDARD 14)

include_directories(.)
aux_source_directory(ycnt/base SOURCE)
aux_source_directory(ycnt/net SOURCE)
aux_source_directory(ycnt/test/base TEST_SOURCE)
aux_source_directory(ycnt/test/net TEST_SOURCE)

add_library(ycnt SHARED ${SOURCE})
add_library(ycnt_static STATIC ${SOURCE})
target_link_libraries(ycnt PUBLIC pthread)
target_link_libraries(ycnt_static PUBLIC pthread)
set_target_properties(ycnt_static PROPERTIES OUTPUT_NAME "ycnt")

include(GoogleTest)
link_libraries(gtest gtest_main)
add_executable(ycnt_test ${SOURCE} ${TEST_SOURCE})
target_link_libraries(
        ycnt_test
        PUBLIC
        gtest
        gtest_main
        ycnt)
gtest_discover_tests(ycnt_test)

add_executable(logger_bench ycnt/test/bench/Logging_bench.cpp)
target_link_libraries(
        logger_bench
        PUBLIC
        ycnt
)