#include <malloc.h>
#include <string.h>

#include "fifo.h"

#define LOCK(fifo)   pthread_mutex_lock(&fifo->mutex)
#define UNLOCK(fifo) pthread_mutex_unlock(&fifo->mutex)

#define is_full(fifo)  (fifo->storedBytes >= fifo->bufferSize)
#define is_empty(fifo) (fifo->storedBytes == 0)

int fifo_create(fifo_t* fifo, uint16_t count, size_t size) {
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

void fifo_free(fifo_t* fifo) {
    free(fifo->buffer);
    pthread_mutex_destroy(&fifo->mutex);
}

void fifo_close(fifo_t* fifo) {
    fifo->closed = true;
}

int fifo_put(fifo_t* fifo, const void* item) {
    // fifo is full, return 1 and do not add
    LOCK(fifo);
    if (is_full(fifo)) {
        UNLOCK(fifo);
        return 1;
    }

    // write to buffer and move ptr
    memcpy(fifo->buffer + fifo->writeOffset, item, fifo->itemSize);
    fifo->writeOffset += fifo->itemSize;
    if (fifo->writeOffset >= fifo->bufferSize) fifo->writeOffset = 0;

    fifo->storedBytes += fifo->itemSize;
    UNLOCK(fifo);
    return 0;
}

int fifo_get(fifo_t* fifo, void* item) {
    LOCK(fifo);
    if (is_empty(fifo)) {
        UNLOCK(fifo);
        return 1;
    }

    // read from buffer and move ptr
    memcpy(item, fifo->buffer + fifo->readOffset, fifo->itemSize);
    fifo->readOffset += fifo->itemSize;
    if (fifo->readOffset >= fifo->bufferSize) fifo->readOffset = 0;

    fifo->storedBytes -= fifo->itemSize;
    UNLOCK(fifo);
    return 0;
}