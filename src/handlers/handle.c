// SPDX-License-Identifier: GPL-2.0
//
// vim: set ts=8 sw=8 noet tw=80 cc=80 fo+=t :

/**
 * @file      handle.c
 * @author    midnight walker
 * @brief     Handler registry, dispatch control, and default-handler setup.
 * @version   0.1
 * @date      2026-08-29
 *
 * @details   This file keeps the global handler list, validates handler
 *            configuration, resolves the appropriate handler for CLI input,
 *            and coordinates execution and cleanup.
 *
 * @copyright GNU General Public License v2.0
 */

#define prfx_fmt "handle: "

#include "clicntl.h"

struct cli_ctxt_struct {
    const handler_t *default_handler;
    struct lhead *lhandlers;
    struct cmd_struct cmd;
};

extern handler_t kfgx_default_handler;

static struct cli_ctxt_struct cli_ctx = {
    .default_handler = NULL,
    .lhandlers = NULL,
};

static int check_cli_ctx()
{
    if (!cli_ctx.lhandlers) {
        pr_error(
            "handlers list not initialized, lhandlers=%p",
            (void *)cli_ctx.lhandlers);
        return -1;
    }

    if (!cli_ctx.lhandlers->head) {
        pr_debug(
            "handlers list head not initialized, head=%p",
            (void *)cli_ctx.lhandlers->head);
        return -1;
    }

    if (!cli_ctx.default_handler) {
        pr_error("no default handler set");
        return -1;
    }

    return 0;
}
static int check_register_ctx(const handler_t *);
static int kfgx_init_handlers(void);
static int kfgx_free_handlers(void);
static int kfgx_resolve_handler(void);

int check_handler(const handler_t *h)
{
    handler_t *h1;

    if (!h) {
        pr_warn("handler=%p", (void *)h);
        return HANDLER_BAD_CONF;
    }

    if (!h->name) {
        pr_error("handler must have a valid name string");
        return HANDLER_BAD_CONF;
    }

    if (!h->action) {
        pr_warn("handler '%s' has a NULL action callback", h->name);
        return HANDLER_BAD_CONF;
    }

    if (h->prio < HANDLER_MIN_PRIO || h->prio > HANDLER_MAX_PRIO) {
        pr_warn("handler '%s' has invalid priority %d", h->name, h->prio);
        return HANDLER_BAD_CONF;
    }

    // check before use cli_ctx.lhandlers->head
    // because endpoint program can use this expose function to test his own
    // handler before register it or to ensure it's still really register
    if (check_cli_ctx())
        return -1;

    foreach_node(h1, cli_ctx.lhandlers->head)
    {
        if (h1 == h) {
            pr_info("handler '%s' is already registered", h->name);
            return HANDLER_REGISTERED;
        }
        if (h1->name && h->name && 0 == strcmp(h1->name, h->name)) {
            pr_info("handler '%s' is already registered", h->name);
            return HANDLER_REGISTERED;
        }
    }

    return HANDLER_UNREGISTERED;
}

int set_default_handler(const handler_t *a)
{
    if (!a) {
        pr_warn("attempted to set NULL as default handler");
        return -1;
    }

    if (HANDLER_REGISTERED != check_handler(a))
        return -1;

    cli_ctx.default_handler = a;
    pr_info("default handler successfully set to '%s'", a->name);

    return 0;
}

static inline int kfgx_execute_handler_impl(handler_action_t act, opt_t *opt)
{
    return act(opt);
}

static int kfgx_execute_handler(void)
{
    opt_t *opt = NULL;

    if (!cli_ctx.cmd.handler || !cli_ctx.cmd.handler->action) {
        pr_error(
            "handler=%p, handler->action=%p",
            (void *)cli_ctx.cmd.handler,
            cli_ctx.cmd.handler ? (void *)cli_ctx.cmd.handler->action : NULL);
        return -1;
    }

    if (cli_ctx.cmd.handler->ltokens && cli_ctx.cmd.handler->ltokens->opt &&
        cli_ctx.cmd.handler->ltokens->opt->l_opt) {
        opt = cli_ctx.cmd.handler->ltokens->opt;
    } else {
        pr_debug("no tokens matched (ltokens is NULL)");
    }

    return kfgx_execute_handler_impl(cli_ctx.cmd.handler->action, opt);
}

static int kfgx_handle_impl(void)
{
    int ret = -1;

    if (kfgx_resolve_handler())
        goto end;

    // Initialize handler options
    cli_ctx.cmd.handler->init_opt(cli_ctx.cmd.handler);

    // Tokenize CLI arguments
    if (kfgx_cli_parser(&cli_ctx.cmd))
        goto end;

    ret = kfgx_execute_handler();

end:
    // cleanup all
    kfgx_token_free(&cli_ctx.cmd.handler->ltokens);
    cli_ctx.cmd.handler->ltokens = NULL; // avoiding the UAF
    kfgx_free_handlers();

    return ret;
}

/**
 * @brief Processes and executes a CLI command payload.
 *
 * Handles the complete lifecycle of a CLI command execution: validates command
 * arguments, resolves the appropriate handler (falling back to @c
 * cli_ctx.default_handler if unspecified or lookup fails), initializes
 * handler-specific options, tokenizes inputs, executes the payload, and
 * performs comprehensive memory cleanup.
 *
 * @return 0 on successful processing and execution.
 * @return -1 if validation fails, handler resolution fails without a default,
 * or token parsing fails.
 *
 * @note This function guarantees resource teardown by releasing the token list
 *       (@c ltokens) and tearing down registered handlers via @ref
 * kfgx_free_handlers before returning upon successful execution.
 */
int handle(void)
{
    if (check_cli_args(&cli_ctx.cmd) || check_cli_ctx())
        return -1;

    return kfgx_handle_impl();
}

//----------| handlers registering

static int check_register_ctx(const handler_t *h)
{
    if (HANDLER_UNREGISTERED != check_handler(h))
        return -1;

    return 0;
}

static int kfgx_register_handler_impl(handler_t *h)
{
    if (prio_list_insert(cli_ctx.lhandlers, h)) {
        pr_warn("failed to insert handler '%s'", h->name);
        return -1;
    }
    pr_info("registered handler '%s' with priority=%d", h->name, h->prio);
    pr_debug("handlers list size=%zu", cli_ctx.lhandlers->size);

    return 0;
}

static int kfgx_unregister_handler_impl(handler_t *h)
{
    if (!prio_list_del(cli_ctx.lhandlers, h)) {
        pr_warn("failed to unregister handler '%s'", h->name);
        return -1;
    }
    pr_info("unregistered handler '%s'", h->name);
    pr_debug("handlers list size=%zu", cli_ctx.lhandlers->size);

    return 0;
}

int register_handler(handler_t *h)
{
    if (check_register_ctx(h))
        return -1;

    return kfgx_register_handler_impl(h);
}

int unregister_handler(handler_t *h)
{
    if (check_register_ctx(h))
        return -1;

    return kfgx_unregister_handler_impl(h);
}

//----------| internal handlers list operations: expose api init_handling

static int kfgx_init_handlers(void)
{
#ifdef STRICT_MODE
    if (cli_ctx.lhandlers)
        pr_warn("suspicious state, handlers list head must be null before his "
                "initialization");
#endif

    cli_ctx.lhandlers = init_prio_list();
    if (cli_ctx.lhandlers)
        return 0;

    pr_error("failed to initialize handlers list");
    return -1;
}

int init_handling(const int argc, const char **argv)
{
    pr_debug("init handling");
    if (kfgx_init_handlers())
        return -1;

    if (prio_list_insert(
            cli_ctx.lhandlers, (handler_t *)&kfgx_default_handler)) {
        pr_error("failed to register '%s'", kfgx_default_handler.name);
    }
    pr_info(
        "registered '%s' with priority %d ",
        kfgx_default_handler.name,
        kfgx_default_handler.prio);

    cli_ctx.default_handler = &kfgx_default_handler;
    pr_info("set '%s' as default handler", kfgx_default_handler.name);

    // init internal cmd structure with apropriate user command line agrs
    cli_ctx.cmd.handler = NULL;
    cli_ctx.cmd.args_nr = argc - 1;           // Exclude the program name
    cli_ctx.cmd.args_set = (char **)argv + 1; // Skip the program name

    return 0;
}

static int kfgx_free_handlers(void)
{
    if (!cli_ctx.lhandlers)
        return 0;

    /* Pop continuous elements until empty to safely avoid broken 'next'
     * pointers */
    handler_t *h;
    while ((h = prio_list_pop(cli_ctx.lhandlers)) != NULL) {
        pr_info("unregistered handler '%s'", h->name);
    }

    free_prio_list(cli_ctx.lhandlers);
    cli_ctx.lhandlers = NULL;
    pr_debug("freed handlers list");
    return 0;
}

/**
 * @brief Resolve the handler matching the current command arguments.
 *
 * @return 0 when a matching handler is found, otherwise -1.
 */
static int kfgx_resolve_handler(void)
{
    handler_t *h;

    h = cli_ctx.cmd.handler;

    if (h) {
        pr_debug(
            "suspicious state, cli_ctx.cmd.handler=%p but it should be NULL",
            (void *)h);
    }

    foreach_node(h, cli_ctx.lhandlers->head)
    {
        if (cli_ctx.cmd.args_set && cli_ctx.cmd.args_set[0] && h->name &&
            !strcmp(h->name, cli_ctx.cmd.args_set[0])) {
            cli_ctx.cmd.handler = h;
            return 0;
        }
    }

    pr_info(
        "no matching handler found for '%s'",
        cli_ctx.cmd.args_set ? cli_ctx.cmd.args_set[0] : "(null)");

    // trying to set it to the default handler
    // don't need it in general because we check it before in handle
    // with check cli ctx
#ifdef STRICT_MODE
    if (!cli_ctx.default_handler) {
        pr_fatal("default handler not set");
        return -1;
    }
#endif

    cli_ctx.cmd.handler = (handler_t *)cli_ctx.default_handler;
    return 0;
}

// --------- shell completions completions
int generate_bash_completions()
{
    handler_t *h;

    if (!cli_ctx.lhandlers || !cli_ctx.lhandlers->head) {
        pr_debug("no handler set");
        return -1;
    }

    if (init_bash_completions_file()) {
        pr_error("bash completion file initialization failed");
        return -1;
    }

    foreach_node(h, cli_ctx.lhandlers->head)
    {
        if (h == &kfgx_default_handler || (h->name && strchr(h->name, ' '))) {
            continue;
        }
        // consume high memory, initialize all handlers options
        h->init_opt(h);
        pr_info("generate bash completions for handler '%s'", h->name);
        if (bash_completions(h)) {
            pr_error("failed to generate bash completions files");
            return -1;
        }
        kfgx_token_free(&h->ltokens);
    }
    return 0;
}

int generate_sh_completions()
{
    handler_t *h;

    if (!cli_ctx.lhandlers || !cli_ctx.lhandlers->head) {
        pr_debug("no handler set");
        return -1;
    }

    if (init_sh_completions_file()) {
        pr_error("sh completion file initialization failed");
        return -1;
    }

    foreach_node(h, cli_ctx.lhandlers->head)
    {
        if (h == &kfgx_default_handler || (h->name && strchr(h->name, ' '))) {
            continue;
        }
        // consume high memory, initialize all handlers options
        h->init_opt(h);
        pr_info("generate bash completions for handler '%s'", h->name);
        if (sh_completions(h)) {
            pr_error("failed to generate sh completions files");
            return -1;
        }
        kfgx_token_free(&h->ltokens);
    }
    return 0;
}