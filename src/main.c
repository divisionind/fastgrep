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
#include <stdint.h>
#include <stdlib.h>
#include <pthread.h>
#include <malloc.h>
#include <string.h>
#include <unistd.h>

#ifndef __MINGW32__
#include <argp.h>
#else
#include "fastgrep-mingw.h"
#endif

#include "strfifo.h"
#include "stringbuilder.h"

#define COLOR(x) ("\033[" x "m")
#define RESET           COLOR("")
#define COLOR_HIGHLIGHT COLOR("95")
#define STR_LEN(x)      (sizeof(x) - 1)

#define AFLAG_USE_COLOR     (1u<<1u)
#define AFLAG_PREVIEW_MATCH (1u)

#define CYGWIN_PREFIX "/cygdrive/"

typedef struct {
    char* query;
    int fifoSize;
    int maxFileDesc;
    int directoryTrim;
    long threads;
    char* directory;
    unsigned int flags;
    int previewBounds;
} arguments_t;

const char* argp_program_bug_address  = "<https://github.com/divisionind/fastgrep/issues>";
static const char* program_version    = "fastgrep v" PROJECT_VERSION;
static char program_desc[]            = "Searches for files recursively in a [-d directory] for the ASCII sequence [QUERY].";
static char program_usage[]           = "[QUERY]";

static struct argp_option options[] = {
    {"buffer-size",    's', "256",   0, "Number of file paths to allow as a buffer for consumption by the worker threads"},
    {"file-desc",      'f', "15",    0, "Max open file desc (only for path traversal), the true usage is [N-(worker threads)]"},
    {"trim-paths",     'p', 0,       0, "Do NOT trim the file paths with the current dir"},
    {"threads",        't', "N",     0, "Number of threads to use for scanning, default is N = [(available processors) - 1]"},
    {"directory",      'd', "\".\"", 0, "Directory to scan"},
    {"no-color",       'k', 0,       0, "Disables color in message printout"},
    {"no-preview",     'P', 0,       0, "Disables the previewing of match line. Note: This also disables color"},
    {"extensions",     'e', 0,       0, "Only display results for files ending in the following. Separate extensions using a ',' and no spaces (e.g. \"java,txt,c\")"},
    {"preview-bounds", 'b', "15",    0, "Amount of text on each side of the result to display in the preview"},
    {"version",        'v', 0,       0, "Print program version"},
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
        case 'P':
            arg->flags &= ~AFLAG_PREVIEW_MATCH;
            break;
        case 'k':
            arg->flags &= ~AFLAG_USE_COLOR;
            break;
        case 'b':
            arg->previewBounds = atoi(in);
            break;
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

#ifdef __CYGWIN__
static void fix_cygwin_path(char* path) {
    if (strncmp(CYGWIN_PREFIX, path, STR_LEN(CYGWIN_PREFIX)) == 0) {
        // starts with cygwin prefix

        path[0] = *(path + STR_LEN(CYGWIN_PREFIX)) - 0x20; // drive letter
        path[1] = ':';
        char current;
        int write = 2;
        for (int read = STR_LEN(CYGWIN_PREFIX) + 1; (current = path[read]) != '\0'; read++, write++) {
            path[write] = current == '/' ? '\\' : current;
        }

        path[write] = 0;
    }
}
#endif

static void* task_search(arguments_t* context) {
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
        ssize_t lineLength;

        if (file != NULL) {
            while ((lineLength = getline(&lineBuffer, &lineBufferSize, file)) != EOF) {
                lineN++;

                char* matchStart;
                if ((matchStart = strstr(lineBuffer, query)) != NULL) {
                    // line contained the search param
                    stringbuilder_t* sb = NULL;       // dynamically allocated stringbuilder
                    char* previewOutput;              // start of actual string passed to printf
                    char* outputFormat = "%s:%i\t%s"; // format of the printf

                    if (context->flags & AFLAG_PREVIEW_MATCH) {
                        // calculate preview bounds
                        int64_t startOffset, stopOffset;
                        size_t queryLen = strlen(query);

                        startOffset = matchStart - lineBuffer;
                        stopOffset  = startOffset + queryLen;
                        startOffset -= context->previewBounds;
                        stopOffset  += context->previewBounds;
                        if (startOffset < 0)
                            startOffset = 0;

                        if (stopOffset > lineLength)
                            stopOffset = lineLength;
                        // end of bound calculations

                        // limit characters to decent looking ascii (replacing with spaces)
                        for (size_t i = 0; i < lineLength; i++) {
                            if (lineBuffer[i] < 0x20 || lineBuffer[i] > 0x7E) lineBuffer[i] = 0x20;
                        }

                        size_t previewLength = stopOffset - startOffset;

                        if (context->flags & AFLAG_USE_COLOR) {
                            sb = sb_create(previewLength + 2 + STR_LEN(COLOR_HIGHLIGHT) + STR_LEN(RESET));
                            sb_append(sb, " ", 1);

                            char* lineBuffPos = lineBuffer + startOffset;
                            size_t curOffset; // could be zero
                            sb_append(sb, lineBuffPos, curOffset = (matchStart - lineBuffPos));
                            lineBuffPos += curOffset;

                            sb_append(sb, COLOR_HIGHLIGHT, STR_LEN(COLOR_HIGHLIGHT));
                            sb_append(sb, lineBuffPos, queryLen);
                            lineBuffPos += queryLen;

                            sb_append(sb, RESET, STR_LEN(RESET));
                            sb_append(sb, lineBuffPos, stopOffset - (lineBuffPos - lineBuffer));

                            outputFormat = "\033[1m%s:%i\033[m\t%s";
                        } else {
                            sb = sb_create(previewLength + 2);
                            sb_append(sb, " ", 1);
                            sb_append(sb, lineBuffer + startOffset, previewLength);
                        }

                        // add suffix return+nulterm
                        sb_append(sb, "\n", 1);
                        previewOutput = sb->buffer;
                    } else {
                        previewOutput = "\n";
                    }

                    #ifdef __CYGWIN__
                    if (context->directoryTrim == 0)
                            fix_cygwin_path(filename);
                    #endif

                    printf(outputFormat, filename + context->directoryTrim, lineN, previewOutput);
                    sb_free(sb);
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
 *
 * TODO create impl for --extensions option
 */
int main(int argc, char** argv) {
    arguments_t args;
    memset(&args, 0, sizeof(args));
    args.fifoSize      = 256; // corresponds to ~1MB ram
    args.maxFileDesc   = 15;
    args.threads       = sysconf(_SC_NPROCESSORS_ONLN) - 1;
    args.directory     = ".";
    args.directoryTrim = -1;
    args.flags         = AFLAG_PREVIEW_MATCH | AFLAG_USE_COLOR;
    args.previewBounds = 15;

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
        fprintf(stderr, "invalid processor configuration, 2 or more cores is required\n");
        return 1;
    }

    // init threads
    pthread_t* threads = malloc(sizeof(pthread_t) * args.threads);
    for (int i = 0; i < args.threads; i++) {
        pthread_create(&threads[i], NULL, (void *(*)(void *)) task_search, &args);
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