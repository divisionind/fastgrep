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

set(PROJECT_DESCRIPTION "a multi-threaded tool to search for files containing a pattern")
set(PROJECT_NAME "fastgrep")
set(PROJECT_HOMEPAGE_URL "https://github.com/divisionind/fastgrep")
set(PROJECT_VERSION_MAJOR 1)
set(PROJECT_VERSION_MINOR 1)
set(PROJECT_VERSION_PATCH 5)
set(PROJECT_VERSION "${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}")
set(PROJECT_VENDOR "Division Industries LLC")

if(MINGW)
    message("MINGW build detected, using proper argp port")
    include_directories(libargp-mingw/src)
    file(GLOB_RECURSE MINGW_SOURCES ${PROJECT_SOURCE_DIR}/libargp-mingw/src/*.*)
    list(APPEND MINGW_SOURCES include/fastgrep-mingw.h src/fastgrep-mingw.c)
else()
    set(MINGW_SOURCES)
endif()

add_definitions(-DPROJECT_VERSION="${PROJECT_VERSION}")
include_directories(src include)
add_executable(fastgrep src/main.c src/strfifo.c include/strfifo.h src/stringbuilder.c include/stringbuilder.h ${MINGW_SOURCES})
target_link_libraries(fastgrep pthread)

if(CYGWIN)
    message("CYGWIN build detected, dynamically linking to argp")
    target_link_libraries(fastgrep argp)
endif()

add_custom_target(PACKAGE_ALL COMMAND cpack WORKING_DIRECTORY .)

# packaging info

# common vars
set(CPACK_PACKAGE_NAME ${PROJECT_NAME})
set(CPACK_PACKAGE_DESCRIPTION ${PROJECT_DESCRIPTION})
set(CPACK_PACKAGE_VENDOR ${PROJECT_VENDOR})
set(CPACK_PACKAGE_VERSION_MAJOR "${PROJECT_VERSION_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR "${PROJECT_VERSION_MINOR}")
set(CPACK_PACKAGE_VERSION_PATCH "${PROJECT_VERSION_PATCH}")
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
set(CPACK_RESOURCE_FILE_LICENSE ${PROJECT_SOURCE_DIR}/LICENSE)
set(CPACK_PACKAGE_CONTACT "Andrew Howard <ahoward@divisionind.com>")
set(CPACK_STRIP_FILES 1)

install(TARGETS fastgrep DESTINATION bin COMPONENT core)
#install(FILES "$ENV{CYGWIN_DIR}/bin/cygwin1.dll" DESTINATION bin COMPONENT core)
#install(FILES "$ENV{CYGWIN_DIR}/bin/cygargp-0.dll" DESTINATION bin COMPONENT core)
# only package with windows installers or zips
#install(FILES LICENSE DESTINATION .)

# /usr/share/doc/{package_name}/copyright | needs proper formatting
#install(FILES LICENSE DESTINATION share/doc)

list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)
# todo only include what is currently being built
#include(CPackDEBConfig)
include(CPackNSISConfig)
#include(CPackZIPConfig)
include(CPack)