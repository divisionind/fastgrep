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

#ifndef FASTGREP_STRFIFO_H
#define FASTGREP_STRFIFO_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdbool.h>
#include <pthread.h>

typedef struct {
    void *buffer;
    size_t buffer_size;
    size_t item_size;
    pthread_mutex_t mutex;

    volatile bool closed;
    volatile size_t read_off;
    volatile size_t write_off;
    volatile size_t stored_bytes;
} sfifo_t;

int sfifo_create(sfifo_t *fifo, size_t count, size_t size);

void sfifo_free(sfifo_t *fifo);

void sfifo_close(sfifo_t *fifo);

int sfifo_put(sfifo_t *fifo, const char *item);

int sfifo_get(sfifo_t *fifo, char *item);

#ifdef __cplusplus
}
#endif

#endif //FASTGREP_STRFIFO_H
