cmake_minimum_required(VERSION 3.10)
project(ycnt)

include_directories(.)
aux_source_directory(ycnt/base SOURCE)
aux_source_directory(ycnt/net SOURCE)

set(CMAKE_CXX_STANDARD 14)

add_library(ycnt SHARED ${SOURCE})
add_library(ycnt_static STATIC ${SOURCE})

set_target_properties(ycnt_static PROPERTIES OUTPUT_NAME "ycnt")