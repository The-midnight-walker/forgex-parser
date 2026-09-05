// SPDX-License-Identifier: GPL-2.0
//
// vim: set ts=8 sw=8 noet tw=80 cc=80 fo+=t :

/**
 * @file      my_handler.c
 * @author    midnight walker
 * @brief
 * @version   0.1
 * @date      2026-09-05
 *
 *@brief Initializes the CLI context with default values before handling starts.
 * Initializes the CLI context structures with default values to ensure that
 * the context is properly initialized and not empty when the handling process
 *begins.
 *
 * @copyright GNU General Public License v2.0
 */

#define prfx_fmt "handle: "

#include "clicntl.h"

static int kfgx_default_handler_init_options(handler_t *h)
{
    (void)h;

    pr_debug("");
    return 0;
}

static int kfgx_default_handler_action(opt_t *options)
{
    (void)options;

    printf("default handle, no default handler set.");
    pr_debug("");
    return 0;
}

// default handler
handler_t kfgx_default_handler = {
    .action = kfgx_default_handler_action,
    .init_opt = kfgx_default_handler_init_options,
    .name = "kfgx default handler",
    .prio = 0,
};