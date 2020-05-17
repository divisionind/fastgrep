cmake_minimum_required(VERSION 3.13)
project(fastgrep C)

set(CMAKE_C_STANDARD 99)

add_executable(fastgrep main.c strfifo.c strfifo.h)
target_link_libraries(fastgrep pthread)

if(CYGWIN)
    target_link_libraries(fastgrep argp)
endif()