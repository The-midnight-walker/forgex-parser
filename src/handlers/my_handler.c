// SPDX-License-Identifier: GPL-2.0
//
// vim: set ts=8 sw=8 noet tw=80 cc=80 fo+=t :

/**
 * @file      my_handler.c
 * @author    jd
 * @brief
 * @version   0.1
 * @date      2026-09-01
 *
 * @details
 *
 * @copyright GNU General Public License v2.0
 */

#include "clicntl.h"

static void my_handler_usage(void)
{
    printf("default handler usage\n");
    printf("Usage: kfgx [options]\n");
    printf("Options:\n");
    printf("  --help, -h     Show this help message\n");
    printf("  --version, -v  Show version information\n");
    printf("  --verbose, -V  Enable verbose output\n");
}

static int my_init_options(handler_t *h)
{
    add_new_option(h, &(opt_t){.l_opt = "--help", .s_opt = "-h"});
    add_new_option(h, &(opt_t){.l_opt = "--version", .s_opt = "-v"});
    add_new_option(h, &(opt_t){.l_opt = "--verbose", .s_opt = "-t"});

    return 0;
}

static int my_handler_action(opt_t *options)
{
    opt_t *o;

    if (!options) {
        my_handler_usage();
    } else {
        foreach_node(o, options)
        {
            pr_debug("%s ", o->l_opt);
            if (HAVE_OPTION(o->l_opt, "--help"))
                my_handler_usage();

            if (HAVE_OPTION(o->l_opt, "--verbose"))
                printf("we are on verbose mode\n");

            if (HAVE_OPTION(o->l_opt, "--version"))
                printf("we are on version 1.0.0\n");
        }
    }
    return 0;
}

// default handler
static handler_t my_handler = {
    .action = my_handler_action,
    .init_opt = my_init_options,
    .name = "default",
    .prio = -1,
    .ltokens = NULL,
    .next = NULL,
};

// default handler
int my_handler_init()
{
    // registering our handler
    if (register_handler(&my_handler)) {
        printf("failed to register default handler");
        return -1;
    }

    // We should set a default handler otherwise , program running failed
    if (set_default_handler(&my_handler)) {
        printf("failed to set default handler");
        return -1;
    }

    return 0;
}