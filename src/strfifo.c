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

#include <malloc.h>
#include <string.h>

#include "strfifo.h"

#define LOCK(fifo)   pthread_mutex_lock(&fifo->mutex)
#define UNLOCK(fifo) pthread_mutex_unlock(&fifo->mutex)

#define is_full(fifo)  (fifo->storedBytes >= fifo->bufferSize)
#define is_empty(fifo) (fifo->storedBytes == 0)

int sfifo_create(sfifo_t* fifo, size_t count, size_t size) {
    // initialize buffers
    fifo->bufferSize = count * size;
    fifo->buffer = malloc(fifo->bufferSize);
    if (!fifo->buffer) return 1;

    fifo->itemSize = size;
    fifo->readOffset = 0;
    fifo->writeOffset = 0;
    fifo->storedBytes = 0;
    fifo->closed = false;
    return pthread_mutex_init(&fifo->mutex, NULL);
}

void sfifo_free(sfifo_t* fifo) {
    free(fifo->buffer);
    pthread_mutex_destroy(&fifo->mutex);
}

void sfifo_close(sfifo_t* fifo) {
    fifo->closed = true;
}

int sfifo_put(sfifo_t* fifo, const char* item) {
    // fifo is full, return 1 and do not add
    LOCK(fifo);
    if (is_full(fifo)) {
        UNLOCK(fifo);
        return 1;
    }

    // write to buffer and move ptr
    strcpy(fifo->buffer + fifo->writeOffset, item); // note: no risk of buffer overflow in this impl as the paths will never exceed MAX_PATH, -> strlcpy() or equiv. for other impls
    fifo->writeOffset += fifo->itemSize;            // strcpy instead of memcpy here to avoid copying zeros for speed (e.g. storedBytes now doesnt reflect actual data stored)
    if (fifo->writeOffset >= fifo->bufferSize) fifo->writeOffset = 0;

    fifo->storedBytes += fifo->itemSize;
    UNLOCK(fifo);
    return 0;
}

int sfifo_get(sfifo_t* fifo, char* item) {
    LOCK(fifo);
    if (is_empty(fifo)) {
        UNLOCK(fifo);
        return 1;
    }

    // read from buffer and move ptr
    strcpy(item, fifo->buffer + fifo->readOffset);
    fifo->readOffset += fifo->itemSize;
    if (fifo->readOffset >= fifo->bufferSize) fifo->readOffset = 0;

    fifo->storedBytes -= fifo->itemSize;
    UNLOCK(fifo);
    return 0;
}