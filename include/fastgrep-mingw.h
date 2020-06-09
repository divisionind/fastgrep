/* fastgrep - a multi-threaded tool to search for files containing a pattern
   Copyright (C) 2020, Andrew Howard, <divisionind.com>

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.  */

#ifndef FASTGREP_FASTGREP_MINGW_H
#define FASTGREP_FASTGREP_MINGW_H

#ifdef __MINGW32__
#include <stdio.h>

#include "argp.h"

#define realpath(x, y) _fullpath(x, y, _MAX_PATH)
#define sysconf(x) getprocessors()

size_t getline(char** lineptr, size_t *n, FILE *stream);
long getprocessors();

#endif

#endif //FASTGREP_FASTGREP_MINGW_H
