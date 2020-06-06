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

#define _XOPEN_SOURCE 700
#define _LARGEFILE64_SOURCE
#define _FILE_OFFSET_BITS 64
#include <ftw.h>

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <malloc.h>
#include <string.h>
#include <unistd.h>
#include <argp.h>

#include "strfifo.h"

typedef struct {
    char* query;
    int fifoSize;
    int maxFileDesc;
    int directoryTrim;
    long threads;
    char* directory;
    bool previewMatch;
    bool useColor;
} arguments_t;

const char* argp_program_bug_address  = "<https://github.com/divisionind/fastgrep/issues>";
static const char* program_version    = "fastgrep 1.5";
static char program_desc[]            = "Searches for files recursively in a [-d directory] for the ASCII sequence [QUERY].";
static char program_usage[]           = "[QUERY]";

static struct argp_option options[] = {
    {"buffer-size", 's', "256",   0, "Number of file paths to allow as a buffer for consumption by the worker threads"},
    {"file-desc",   'f', "15",    0, "Max open file desc (only for path traversal), the true usage is [N-(worker threads)]"},
    {"trim-paths",  'p', 0,       0, "Do NOT trim the file paths with the current dir"},
    {"threads",     't', "N",     0, "Number of threads to use for scanning, default is N = [(available processors) - 1]"},
    {"directory",   'd', "\".\"", 0, "Directory to scan"},
    {"no-color",    'k', 0,       0, "Disables color in message printout"},
    {"no-preview",    0, 0,       0, "Disables the previewing of match line"},
    {"extensions",  'e', 0,       0, "Only display results for files ending in the following. Separate extensions using a ',' and no spaces (e.g. \"java,txt,c\")"},
    {"version",     'v', 0,       0, "Print program version"},
    {0}
};

static error_t parse_opt(int key, char* in, struct argp_state* state) {
    arguments_t* arg = (arguments_t*) state->input;
    switch (key) {
        case 's':
            arg->fifoSize = atoi(in);
            break;
        case 'f':
            arg->maxFileDesc = atoi(in);
            break;
        case 'p':
            arg->directoryTrim = 0;
            break;
        case 't':
            arg->threads = atol(in);
            break;
        case 'd':
            arg->directory = in;
            break;
        case 'v':
            printf("%s\n", program_version);
            exit(0);
        case ARGP_KEY_ARG:
            if (state->arg_num >= 1)
                argp_usage(state);
            else
            if (state->arg_num == 0)
                arg->query = in;
            break;
        case ARGP_KEY_END:
            if (state->arg_num < 1)
                argp_usage(state);
            break;
        default:
            return ARGP_ERR_UNKNOWN;
    }
    return 0;
}

static struct argp arg_parser = {options, parse_opt, program_usage, program_desc};
sfifo_t fifo;

static void* task_search(void* arg) {
    arguments_t * context = arg;
    const char* query = context->query;
    char filename[PATH_MAX];

    while (!(fifo.closed && fifo.storedBytes == 0)) {
        if (sfifo_get(&fifo, filename)) {
            continue;
        }
        FILE* file = fopen(filename, "r");
        unsigned int lineN = 0;
        size_t lineBufferSize = 0;
        char* lineBuffer = NULL;

        if (file != NULL) {
            while ((getline(&lineBuffer, &lineBufferSize, file)) != EOF) {
                lineN++;

                if (strstr(lineBuffer, query) != NULL) {
                    // line contained the search param
                    printf("%s:%i\n", filename + context->directoryTrim, lineN);
                }
            }

            free(lineBuffer);
            fclose(file);
        }
    }

    return NULL;
}

static int task_load_file_entry(const char *filename, const struct stat *info, int flag, struct FTW *pathInfo) {
    if (flag == FTW_F) {
        while (sfifo_put(&fifo, filename));
    }
    return 0;
}

/*
 * Create options for:
 * - ignore case / regex
 * - disable color   (impl color first)
 * - disable preview (impl prev first)
 */
int main(int argc, char** argv) {
    arguments_t args;
    memset(&args, 0, sizeof(args));
    args.fifoSize      = 256; // corresponds to ~1MB ram
    args.maxFileDesc   = 15;
    args.threads       = sysconf(_SC_NPROCESSORS_ONLN) - 1;
    args.directory     = ".";
    args.directoryTrim = -1;

    argp_parse(&arg_parser, argc, argv, 0, 0, &args);

    // resolve directory
    char* oldDir = args.directory;
    args.directory = realpath(args.directory, NULL);
    if (args.directory == NULL) {
        fprintf(stderr, "invalid directory: %s\n", oldDir);
        return 1;
    }

    if (args.directoryTrim == -1)
        args.directoryTrim = (int) strlen(args.directory) + 1;

    // done parsing args, create fifo
    if (sfifo_create(&fifo, args.fifoSize, PATH_MAX)) {
        fprintf(stderr, "insufficient memory or other resources\n");
        return 1;
    }

    if (args.threads < 1) {
        fprintf(stderr, "invalid processor configuration\n");
        return 1;
    }

    // init threads
    pthread_t* threads = malloc(sizeof(pthread_t) * args.threads);
    for (int i = 0; i < args.threads; i++) {
        pthread_create(&threads[i], NULL, task_search, &args);
    }

    // iterate files and send them to the fifo
    nftw(args.directory, task_load_file_entry, args.maxFileDesc, FTW_PHYS); // max # open file descriptors, do not follow symlinks (todo maybe allow this? as an option)

    // cleanup
    sfifo_close(&fifo); // ensure it is closed before joining threads
    for (int i = 0; i < args.threads; i++) {
        pthread_join(threads[i], NULL);
    }
    sfifo_free(&fifo);
    return 0;
}
