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
    message("MINGW build detected, using argp port https://github.com/divisionind/libargp-mingw")
    include_directories(libargp-mingw/src)
    file(GLOB_RECURSE MINGW_SOURCES ${PROJECT_SOURCE_DIR}/libargp-mingw/src/*.*)
    list(APPEND MINGW_SOURCES src/fastgrep-mingw.h src/fastgrep-mingw.c)
else()
    set(MINGW_SOURCES)
endif()

add_definitions(-DPROJECT_VERSION="${PROJECT_VERSION}")
include_directories(src)
add_executable(fastgrep src/main.c src/strfifo.c src/strfifo.h src/stringbuilder.c src/stringbuilder.h ${MINGW_SOURCES})
target_link_libraries(fastgrep pthread)

add_custom_target(PACKAGE_ALL COMMAND cpack WORKING_DIRECTORY .)

# === PACKAGING INFO ===
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

list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)

install(TARGETS fastgrep DESTINATION bin COMPONENT core)
if(MINGW)
    install(FILES "$ENV{MINGW_DIR}/bin/libwinpthread-1.dll" DESTINATION bin COMPONENT core)
    install(FILES LICENSE DESTINATION . COMPONENT core)
    include(CPackNSISConfig)
else()
    include(CPackDEBConfig)
endif()

# /usr/share/doc/{package_name}/copyright | needs proper formatting
#install(FILES LICENSE DESTINATION share/doc)

#include(CPackZIPConfig)
include(CPack)