// SPDX-License-Identifier: GPL-2.0
//
// vim: set ts=8 sw=8 noet tw=80 cc=80 fo+=t :

/**
 * @file      main.c
 * @author    midnight walker
 * @brief     Main entry point for kernforgex.
 * @version   0.2
 * @date      2026-09-05
 *
 * @details   Initializes CLI context, registers handlers, generates
 * completions, and dispatches the command line execution.
 *
 * @copyright GNU General Public License v2.0
 */

#include <stdio.h>
#include <string.h>

#define STRICT_MODE

#include "clicntl.h"
#include "my_handler.h"

int main(int argc, char *argv[])
{

    // init command line handling
    init_handling(argc, (const char **)argv);

    // init our handler
    // insert our default handler in our case it is under src/handlers
    if (my_handler_init()) {
        return -1;
    }

    // avoid this cause high memory on free area
    generate_bash_completions();

    // handling
    if (handle()) {
        fprintf(stderr, "Error: Parsing failed\n");
        return -1;
    }

    return 0;
}