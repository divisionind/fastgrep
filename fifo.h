#ifndef FASTGREP_FIFO_H
#define FASTGREP_FIFO_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <pthread.h>

typedef struct {
    void* buffer;
    size_t bufferSize;
    size_t itemSize;
    pthread_mutex_t mutex;

    volatile bool closed;
    volatile uint16_t readOffset;
    volatile uint16_t writeOffset;
    volatile size_t storedBytes;
} fifo_t;

int fifo_create(fifo_t* fifo, uint16_t count, size_t size);
void fifo_free(fifo_t* fifo);
void fifo_close(fifo_t* fifo);
int fifo_put(fifo_t* fifo, const void* item);
int fifo_get(fifo_t* fifo, void* item);

#ifdef __cplusplus
}
#endif

#endif //FASTGREP_FIFO_H
