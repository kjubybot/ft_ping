#include "ft_ping.h"
#include <ctype.h>
#include <stdio.h>

static int parse_arg_int(int *dest, char *s) {
    if (s == 0) {
        return -1;
    }

    for (size_t i = 0; s[i]; i++) {
        if (!(isdigit(s[i]) || isspace(s[i]))) {
            return -1;
        }
    }
    if (sscanf(s, "%d", dest) != 1) {
        return -1;
    }

    return 0;
}

static int parse_arg_interval(struct timeval *dest, char *s) {
    if (s == 0) {
        return -1;
    }

    size_t dots = 0;
    for (size_t i = 0; s[i]; i++) {
        if (!(isdigit(s[i]) || isspace(s[i]) || s[i] == '.')) {
            return -1;
        }
        if (s[i] == '.') {
            dots++;
            if (dots >= 2) {
                return -1;
            }
        }
    }

    if (dots > 0) {
        sscanf(s, "%ld.%ld", &dest->tv_sec, &dest->tv_usec);
        dest->tv_usec *= 100000;
    } else {
        sscanf(s, "%ld", &dest->tv_sec);
        dest->tv_usec = 0;
    }

    return 0;
}

int parse_opts(char **argv, opts_t *opts) {
    int i, last = 1;

    opts->ttl = DEFAULT_TTL;
    opts->count = -1;
    opts->interval.tv_sec = 1;

    for (i = 1; argv[i]; i++) {
        char *s = argv[i];
        if (s[0] == '-') {
            while (*++s) {
                switch (*s) {
                    case 'c':
                        if (*(s+1)) {
                            fputs(PROG_NAME": could not parse argument for count\n", stderr);
                            return -1;
                        }
                        i++;
                        if (parse_arg_int(&opts->count, argv[i])) {
                            fprintf(stderr, PROG_NAME": could not parse %s as count\n", argv[i]);
                            return -1;
                        }
                        if (opts->count < 1) {
                            fprintf(stderr, PROG_NAME": count %d is too small\n", opts->count);
                            return -1;
                        }
                        break;
                    case 'D':
                        opts->timestamp = 1;
                        break;
                    case 'h':
                        opts->help = 1;
                        break;
                    case 'i':
                        if (*(s+1)) {
                            fputs(PROG_NAME": could not parse argument for interval\n", stderr);
                            return -1;
                        }
                        i++;
                        if (parse_arg_interval(&opts->interval, argv[i])) {
                            fprintf(stderr, PROG_NAME": could not parse %s as interval\n", argv[i]);
                            return -1;
                        }
                        break;
                    case 'q':
                        opts->quiet = 1;
                        break;
                    case 't':
                        if (*(s+1)) {
                            fputs(PROG_NAME": could not parse argument for ttl\n", stderr);
                            return -1;
                        }
                        i++;
                        if (parse_arg_int(&opts->ttl, argv[i])) {
                            fprintf(stderr, PROG_NAME": could not parse %s as ttl\n", argv[i]);
                            return -1;
                        }
                        break;
                    case 'v':
                        opts->verbose = 1;
                        break;
                }
            }
        } else {
            char *tmp = argv[i];
            argv[i] = argv[last];
            argv[last] = tmp;
            last++;
        }
    }

    if (last == 1 && opts->help == 0) {
        fputs(PROG_NAME": not enough arguments\n", stderr);
        return -1;
    }

    return 0;
}
