cmake_minimum_required(VERSION 3.6)
project(lcc_zach)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_BUILD_TYPE DEBUG)

add_library(lccall string.c alloc.c)
# include_directories(include)

add_executable(lcc lcc.c)
target_link_libraries(lcc lccall)
