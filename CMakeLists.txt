cmake_minimum_required(VERSION 3.10)
project(ycnt)

set(CMAKE_CXX_STANDARD 14)

add_library(ycnt SHARED library.cpp library.h)