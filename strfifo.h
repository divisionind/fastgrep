#ifndef FASTGREP_STRFIFO_H
#define FASTGREP_STRFIFO_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdbool.h>
#include <pthread.h>

typedef struct {
    void* buffer;
    size_t bufferSize;
    size_t itemSize;
    pthread_mutex_t mutex;

    volatile bool closed;
    volatile size_t readOffset;
    volatile size_t writeOffset;
    volatile size_t storedBytes;
} sfifo_t;

int sfifo_create(sfifo_t* fifo, size_t count, size_t size);
void sfifo_free(sfifo_t* fifo);
void sfifo_close(sfifo_t* fifo);
int sfifo_put(sfifo_t* fifo, const char* item);
int sfifo_get(sfifo_t* fifo, char* item);

#ifdef __cplusplus
}
#endif

#endif //FASTGREP_STRFIFO_H
