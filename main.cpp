/*
 * Copyright (c) 2013, Todd E Brandt <tebrandt@frontier.com>.
 *
 * This program is licensed under the terms and conditions of the
 * GPL License, version 2.0.  The full text of the GPL License is at
 * http://www.gnu.org/licenses/gpl-2.0.html
 *
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <signal.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <fcntl.h>

#include "defines.h"
#include "display.h"

int verbose = DEFAULT_VERBOSE;

static void help(const char *appname)
{
    printf("%s [options]\n\n"
           "  -h, --help                            Show this help\n"
           "      --version                         Show version\n\n"
           "  -v, --verbose                         Enable verbose operations\n\n"
           "  -f, --tlefile=VOLUME                  Specify the TLE data file\n\n"
           ,
           appname);
}

int main(int argc, char *argv[]) {
    int ret = 1, ch;
    char *tlefile = NULL;

    enum {
        ARG_VERSION = 256,
        ARG_TLEFILE
    };

    static const struct option long_options[] = {
        {"version",      0, NULL, ARG_VERSION},
        {"help",         0, NULL, 'h'},
        {"verbose",      0, NULL, 'v'},
        {"tlefile",      1, NULL, ARG_TLEFILE},
        {NULL,           0, NULL, 0}
    };

    while ((ch = getopt_long(argc, argv, "hv", long_options, NULL)) != -1) {
        switch (ch) {
            case 'h' :
                help(PACKAGE_NAME);
                ret = 0;
                goto quit;
            case ARG_VERSION:
                printf("%s\n", PACKAGE_STRING);
                ret = 0;
                goto quit;
            case 'v':
                verbose = 1;
                break;
            case ARG_TLEFILE:
                tlefile = strdup(optarg);
                break;
            default:
                goto quit;
        }
    }

    glutInit(&argc, argv);
    Display::create();
    glutMainLoop();

quit:
    return ret;
}
