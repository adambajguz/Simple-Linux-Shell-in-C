cmake_minimum_required(VERSION 3.10)
project(Shell C)

set(CMAKE_C_STANDARD 11)
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -pedantic-errors")

add_executable(Shell main.c history.c history.h filemanip.c filemanip.h parser.c parser.h config.h)