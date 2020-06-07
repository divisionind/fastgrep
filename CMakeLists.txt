# fastgrep - a multi-threaded tool to search for files containing a pattern
# Copyright (C) 2020, Andrew Howard, <divisionind.com>
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <https://www.gnu.org/licenses/>.

cmake_minimum_required(VERSION 3.13)
project(fastgrep C)

set(CMAKE_C_STANDARD 99)

add_executable(fastgrep main.c strfifo.c strfifo.h stringbuilder.c stringbuilder.h)
target_link_libraries(fastgrep pthread)

if(CYGWIN)
    target_link_libraries(fastgrep argp)
endif()