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
#include <sys/stat.h>

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

#define AFLAG_FROM_STDIN    (1<<2)
#define AFLAG_USE_COLOR     (1<<1)
#define AFLAG_PREVIEW_MATCH (1)

struct {
    char *query;
    int fifoSize;
    int maxFileDesc;
    int directoryTrim;
    long threads;
    char *directory;
    unsigned int flags;
    int previewBounds;
    char **extensions;
    int nExtensions;
} args;

const char *argp_program_bug_address  = "<https://github.com/divisionind/fastgrep/issues>";
static const char *program_version    = "fastgrep v" PROJECT_VERSION;
static char program_desc[]            = "Searches for files recursively in a [-d directory] for the ASCII sequence [QUERY].";
static char program_usage[]           = "[QUERY]";

static struct argp_option options[] = {
    {"buffer-size",    's', "256",    0, "Number of file paths to allow as a buffer for consumption by the worker threads"},
    {"file-desc",      'f', "15",     0, "Max open file desc (only for path traversal), the true usage is [N-(worker threads)]"},
    {"trim-paths",     'p', 0,        0, "Do NOT trim the file paths with the current dir"},
    {"threads",        't', "N",      0, "Number of threads to use for scanning, default is N = [(available processors) - 1]"},
    {"directory",      'd', "\".\"",  0, "Directory to scan"},
    {"no-color",       'k', 0,        0, "Disables color in message printout"},
    {"no-preview",     'P', 0,        0, "Disables the previewing of match line. Note: This also disables color"},
    {"extensions",     'e', "ext,..", 0, "Only display results for files ending in the following. Separate extensions using a ',' and no spaces (e.g. \"java,txt,c\")"},
    {"preview-bounds", 'b', "15",     0, "Amount of text on each side of the result to display in the preview"},
    {"version",        'v', 0,        0, "Print program version"},
    {"stdin",          'i', 0,        0, "Search files provided from stdin rather than a directory"},
    {0}
};

static error_t parse_opt(int key, char *in, struct argp_state *state) {
    switch (key) {
        case 's':
            args.fifoSize = atoi(in);
            break;
        case 'f':
            args.maxFileDesc = atoi(in);
            break;
        case 'p':
            args.directoryTrim = 0;
            break;
        case 't':
            args.threads = atol(in);
            break;
        case 'd':
            args.directory = in;
            break;
        case 'v':
            printf("%s\n", program_version);
            exit(0);
        case 'P':
            args.flags &= ~AFLAG_PREVIEW_MATCH;
            break;
        case 'k':
            args.flags &= ~AFLAG_USE_COLOR;
            break;
        case 'i':
            args.flags |= AFLAG_FROM_STDIN;
            break;
        case 'e': {
            int count = 1;
            char current;
            for (int i = 0; (current = in[i]) != '\0'; i++) {
                if (current == ',') count++;
            }

            if (count) {
                args.extensions = malloc(sizeof(void*) * (args.nExtensions = count));

                char* str = strtok(in, ",");
                for (int i = 0; str != NULL; i++) {
                    args.extensions[i] = str;
                    str = strtok(NULL, ",");
                }
            }
            break;
        }
        case 'b':
            args.previewBounds = atoi(in);
            break;
        case ARGP_KEY_ARG:
            if (state->arg_num >= 1)
                argp_usage(state);
            else
            if (state->arg_num == 0)
                args.query = in;
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

static void *task_search(void *context) {
    (void) context;

    char filename[PATH_MAX];

    while (!(fifo.closed && fifo.stored_bytes == 0)) {
        // aquire file from fifo
        if (sfifo_get(&fifo, filename)) {
            continue;
        }

        // verify extensions match specified before reading file
        if (args.nExtensions) {
            char* extensionIndex = strrchr(filename, '.');
            if (!extensionIndex) continue;

            extensionIndex++;
            int i = 0;
            do {
                if (!strcmp(extensionIndex, args.extensions[i])) {
                    // one of the ext matched, continue
                    goto search_file;
                }

                i++;
            } while (i < args.nExtensions);

            continue;
        }

        search_file: ;
        FILE* file = fopen(filename, "r");
        unsigned int lineN = 0;
        size_t lineBufferSize = 0;
        char* lineBuffer = NULL;
        ssize_t lineLength;

        if (file != NULL) {
            while ((lineLength = getline(&lineBuffer, &lineBufferSize, file)) != EOF) {
                lineN++;

                char* matchStart;
                if ((matchStart = strstr(lineBuffer, args.query)) != NULL) {
                    // line contained the search param
                    stringbuilder_t* sb = NULL;       // dynamically allocated stringbuilder
                    char* previewOutput;              // start of actual string passed to printf
                    char* outputFormat = "%s:%i\t%s"; // format of the printf

                    if (args.flags & AFLAG_PREVIEW_MATCH) {
                        // calculate preview bounds
                        int64_t startOffset, stopOffset;
                        size_t queryLen = strlen(args.query);

                        startOffset = matchStart - lineBuffer;
                        stopOffset  = startOffset + queryLen;
                        startOffset -= args.previewBounds;
                        stopOffset  += args.previewBounds;
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

                        if (args.flags & AFLAG_USE_COLOR) {
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

                    #ifdef __MINGW32__
                    mingw_fix_path(filename);
                    #endif

                    printf(outputFormat, filename + args.directoryTrim, lineN, previewOutput);
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
    (void) info;
    (void) pathInfo;

    if (flag == FTW_F) {
        while (sfifo_put(&fifo, filename));
    }
    return 0;
}

/*
 * Create options for:
 * - ignore case / regex (impl through a comparator function in place of current check)
 *
 * TODO add:
 * - replace match with alternative (maybe)
 * - logging / verbose mode (including file counting)
 * - snapping previews (preview snaps to the closest space if within certain # chars, will let more whole words come into frame)
 * - space ignoring previews (beginning and trailing spaces will be ignored in previews)
 * - option to enable follow symlinks
 * - safe-mallocs/reallocs which redirect to an error proc on fail
 * - extensions filter for excluding certain file types (e.g. --include, --exclude) instead of --extensions
 *
 * BIG: scan mode for strings not by line but by byte-sequence (for binary files)
 *      (also add option to parse escapes in string input, e.g. \xAE)
 */
int main(int argc, char **argv) {
    args.fifoSize      = 256; // corresponds to ~1MB ram
    args.maxFileDesc   = 15;
    args.threads       = sysconf(_SC_NPROCESSORS_ONLN) - 1;
    args.directory     = ".";
    args.directoryTrim = -1;
    args.flags         = AFLAG_PREVIEW_MATCH | AFLAG_USE_COLOR;
    args.previewBounds = 15;

    #ifdef __MINGW32__
    mingw_enable_color();
    #endif

    argp_parse(&arg_parser, argc, argv, 0, 0, NULL);

    // resolve directory
    args.directory = realpath(args.directory, NULL);
    if (args.directory == NULL) {
        fprintf(stderr, "invalid directory\n");
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
        pthread_create(&threads[i], NULL, task_search, NULL);
    }

    // iterate files and send them to the fifo
    if (args.flags & AFLAG_FROM_STDIN) {
        // just ignore dir trim all together for now
        args.directoryTrim = 0;

        char* lineBuffer = NULL;
        size_t lineBufferSize = 0;
        ssize_t lineLen;

        while ((lineLen = getline(&lineBuffer, &lineBufferSize, stdin)) != EOF) {
            lineBuffer[lineLen - 1] = 0;

            struct stat fstatus;
            if (!stat(lineBuffer, &fstatus) && S_ISREG(fstatus.st_mode)) { // todo add symlinks later (if add follow symlinks opt)
                while (sfifo_put(&fifo, lineBuffer));
            }
        }

        free(lineBuffer);
    } else {
        nftw(args.directory, task_load_file_entry, args.maxFileDesc, FTW_PHYS); // max # open file descriptors, do not follow symlinks (todo maybe allow this? as an option)
    }

    // cleanup
    sfifo_close(&fifo); // ensure it is closed before joining threads
    for (int i = 0; i < args.threads; i++) {
        pthread_join(threads[i], NULL);
    }
    sfifo_free(&fifo);
    free(args.extensions);
    return 0;
}