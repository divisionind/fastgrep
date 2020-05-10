cmake_minimum_required(VERSION 3.13)
project(fastgrep C)

set(CMAKE_C_STANDARD 99)

add_executable(fastgrep main.c fifo.c fifo.h)
target_link_libraries(fastgrep pthread)