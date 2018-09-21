cmake_minimum_required(VERSION 3.9)
project(huffman)

set(CMAKE_CXX_STANDARD 17)

add_library(huffman_lib
        lib/huffman.h lib/huffman.cpp
        lib/huff_tree.h  lib/huff_tree.cpp)

add_executable(huffman_cli cli.cpp)
target_link_libraries(huffman_cli huffman_lib)

add_executable(huffman_test test.cpp)
target_link_libraries(huffman_test huffman_lib)