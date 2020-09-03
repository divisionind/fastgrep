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

#include <stdlib.h>
#include <string.h>

#include "stringbuilder.h"

stringbuilder_t *sb_create(size_t size) {
    stringbuilder_t *sb = malloc(sizeof(stringbuilder_t));
    if (sb == NULL) return NULL;

    sb->buffer = malloc(size + 1);
    if (sb->buffer == NULL) {
        free(sb);
        return NULL;
    }

    sb->buffer_size = size;
    sb->offset = 0;
    sb->buffer[size] = 0;

    return sb;
}

int sb_append(stringbuilder_t *sb, const char *content, size_t len) {
    size_t newOffset = sb->offset + len;
    if (newOffset > sb->buffer_size) {
        newOffset = sb->buffer_size;
        len = sb->buffer_size - sb->offset;
    }

    memcpy(sb->buffer + sb->offset, content, len);
    sb->offset = newOffset;
    return 0;
}

void sb_free(stringbuilder_t *sb) {
    if (sb != NULL) {
        free(sb->buffer);
        free(sb);
    }
}