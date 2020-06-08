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

#ifndef FASTGREP_FASTGREP_MINGW_H
#define FASTGREP_FASTGREP_MINGW_H

#ifdef __MINGW32__
#include <stdio.h>

#define realpath(x, y) _fullpath(x, y, _MAX_PATH)
#define sysconf(x) getprocessors()

size_t getline(char** lineptr, size_t *n, FILE *stream);
long getprocessors();

/*
 * partial argp mingw impl
 */
#define ARGP_KEY_ARG            0
#define ARGP_KEY_ARGS           0x1000006
#define ARGP_KEY_END            0x1000001
#define ARGP_KEY_NO_ARGS        0x1000002
#define ARGP_KEY_INIT           0x1000003
#define ARGP_KEY_FINI           0x1000007
#define ARGP_KEY_SUCCESS        0x1000004
#define ARGP_KEY_ERROR          0x1000005
#define ARGP_ERR_UNKNOWN        7

struct argp_state;

typedef int error_t;
typedef error_t (*argp_parser_t)(int key, char *arg, struct argp_state *state);

struct argp_option {
    const char *name;
    int key;
    const char *arg;
    int flags;
    const char *doc;
    int group;
};

struct argp {
    const struct argp_option *options;
    argp_parser_t parser;
    const char *args_doc;
    const char *doc;
    const struct argp_child *children;
    char *(*help_filter)(int key, const char *text, void *input);
    const char *argp_domain;
};

struct argp_state {
    const struct argp *root_argp;
    int argc;
    char **argv;
    int next;
    unsigned flags;
    unsigned arg_num;
    int quoted;
    void *input;
    void **child_inputs;
    void *hook;
    char *name;
    FILE *err_stream;
    FILE *out_stream;
    void *pstate;
};

extern error_t argp_parse(const struct argp* pargp,
                           int argc, char** argv,
                           unsigned flags, int* arg_index,
                           void* input);
extern void argp_usage(const struct argp_state *state);

#endif

#endif //FASTGREP_FASTGREP_MINGW_H
