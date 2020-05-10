#define _XOPEN_SOURCE 700
#define _LARGEFILE64_SOURCE
#define _FILE_OFFSET_BITS 64
#include <ftw.h>

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <malloc.h>
#include <string.h>

#include "fifo.h"

#define FIFO_SIZE 512
#define SEARCH_THREADS 11
#define MAX_OPEN_DESCRIPTORS 15

typedef struct {
    fifo_t* fifo;
    size_t directoryTrim;
    char query[64];
} search_context_t;

// i hate using a global here but nftw has no way of passing data
// maybe implement a similar version? check speed
fifo_t g_fifo;

void* task_search(void* arg) {
    search_context_t* context = arg;
    fifo_t* fifo = context->fifo;
    const char* query = context->query;
    char* filename;

    do {
        if (fifo_get(fifo, &filename)) {
            continue;
        }
        FILE* file = fopen(filename, "r");
        unsigned int lineN = 0;
        size_t lineBufferSize = 0;
        char* lineBuffer = NULL;

        if (file != NULL) {
            while ((getline(&lineBuffer, &lineBufferSize, file)) != EOF) {
                // lineN - current line; read - length of current line; lineBuffer - actual line string
                lineN++;

                if (strstr(lineBuffer, query) != NULL) {
                    // then the line contained the search param
                    printf("%s:%i\n", filename + context->directoryTrim, lineN);
                }
            }

            free(lineBuffer);
            fclose(file);
        }

        free(filename);
    } while (!fifo->closed);

    return NULL;
}

int task_load_file_entry(const char *filename, const struct stat *info, int flag, struct FTW *pathInfo) {
    if (flag == FTW_F) {
        // entry was a file, add it

        // first create a copy of the filename string as it will be cleaned on function proceed
        unsigned long fileNameLen = strlen(filename) + 1;
        char* fileNameCopy = malloc(fileNameLen);    // TODO maybe reduce malloc's here? test speed vs direct cpy to buffer
        memcpy(fileNameCopy, filename, fileNameLen); // only prob is max_path = 4096 which is more space than I am comfortable with assuming 500 fifo spots, only ~2mb ram but for best eff. i would have to re-structure the fifo util

        // then add the file to the fifo buffer/stream | wait for it to be added successfully
        while (fifo_put(&g_fifo, &fileNameCopy));
    }
    return 0;
}

/*
 * Create options for:
 * - directory trimming
 * - fifo size
 * - max open file desc
 * - search threads
 * - ignore case / regex
 */
int main(int argc, char* argv[]) {
    // TODO impl cmd line parsing using getopt (http://man7.org/linux/man-pages/man3/getopt.3.html)
    if (argc < 2) {
        printf("usage: fastgrep <query> <none:dir>\n");
        return 1;
    }

    // init fifo
    fifo_create(&g_fifo, FIFO_SIZE, sizeof(char*));

    char* potentialDir = argc >= 3 ? argv[2] : ".";
    char* dir = realpath(potentialDir, NULL);
    if (dir == NULL) {
        printf("invalid directory: %s\n", potentialDir);
    }

    printf("Searching \"%s\"...\n", dir);

    // create a context to pass to threads
    search_context_t searchContext;
    searchContext.fifo = &g_fifo;
    searchContext.directoryTrim = strlen(dir) + 1;
//    searchContext.directoryTrim = 0;
    strcpy(searchContext.query, argv[1]);

    // init threads
    pthread_t* threads = malloc(sizeof(pthread_t) * SEARCH_THREADS);
    for (int i = 0; i < SEARCH_THREADS; i++) {
        pthread_create(&threads[i], NULL, task_search, &searchContext);
    }

    // iterate files and send them to the fifo
    // maybe dont use nftw so I can avoid global usage (see performance)
    nftw(dir, task_load_file_entry, MAX_OPEN_DESCRIPTORS, FTW_PHYS); // max # open file descriptors, do not follow symlinks

    // cleanup
    fifo_close(&g_fifo); // ensure it is closed before joining threads
    for (int i = 0; i < SEARCH_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }
    fifo_free(&g_fifo);
    return 0;
}
