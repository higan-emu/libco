# Set OS and processor architecture
cmake_minimum_required(VERSION 3.6)
project(libco)

add_library(libco libco.c)
target_include_directories(libco PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

enable_testing()
add_subdirectory(tests)
