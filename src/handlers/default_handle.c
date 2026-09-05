// SPDX-License-Identifier: GPL-2.0
//
// vim: set ts=8 sw=8 noet tw=80 cc=80 fo+=t :

/**
 * @file      default_handle.c
 * @author    midnight walker
 * @brief     Implementation of the fallback default CLI handler.
 * @version   0.2
 * @date      2026-09-05
 *
 * @details   Initializes the CLI context with default fallback behavior to
 * ensure the command dispatcher has a valid handler even if no custom handler
 *            matches the input arguments.
 *
 * @copyright GNU General Public License v2.0
 */

#define prfx_fmt "handle: "

#include "clicntl.h"

static int kfgx_default_handler_init_options(handler_t *h)
{
    (void)h;

    pr_debug("no options to initialize for default fallback handler");
    return 0;
}

static int kfgx_default_handler_action(opt_t *options)
{
    (void)options;

    printf("default handle: no specific handler matched or set.\n");
    return 0;
}

// default fallback handler
handler_t kfgx_default_handler = {
    .action = kfgx_default_handler_action,
    .init_opt = kfgx_default_handler_init_options,
    .name = "kfgx default handler",
    .prio = 0,
    .ltokens = NULL,
    .next = NULL,
};