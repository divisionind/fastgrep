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

#ifndef FASTGREP_STRINGBUILDER_H
#define FASTGREP_STRINGBUILDER_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>

typedef struct {
    char *buffer;
    size_t buffer_size;
    size_t offset;
} stringbuilder_t;

stringbuilder_t *sb_create(size_t size);

int sb_append(stringbuilder_t *sb, const char *content, size_t len);

void sb_free(stringbuilder_t *sb);

#ifdef __cplusplus
}
#endif

#endif //FASTGREP_STRINGBUILDER_H
