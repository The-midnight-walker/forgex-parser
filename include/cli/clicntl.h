// SPDX-License-Identifier: GPL-2.0
//
// vim: set ts=8 sw=8 noet tw=80 cc=80 fo+=t :

#ifndef INCLUDE_CLICNTL_H
#define INCLUDE_CLICNTL_H

#include <errno.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

// Consolidates the full core system architecture into a unified master header.
// #include "clicntl"
#include "debug.h"
#include "handlers.h"
#include "prio_lists.h"
#include "shell_completions.h"

#define HAVE_OPTION(option, l_opt) (!strcmp((option), (l_opt)))
#define MATCH_L_OPT(opt, arg) (!strcmp((opt), (arg)))
#define MATCH_S_OPT(opt, arg) (!strcmp((opt), (arg)))

/* Token State Flags */
#define TOKEN_VALID 0x00
#define TOKEN_DIRTY 0x01

typedef struct handler handler_t;
typedef int (*init_options_t)(handler_t *h);

typedef struct kfgx_opt {
    char *l_opt; /* "--foo" long option */
    char *s_opt; /* "-f" short option */
    char *value;
    struct kfgx_opt *next;
} opt_t;

typedef struct kfgx_token {
    opt_t *opt;
    unsigned int free : 1;
    unsigned int match : 1;
    token_t *next;
} token_t;

struct kfgx_cmd_struct {
    handler_t *handler;
    int args_nr;
    char **args_set;
};

int kfgx_cli_parser(struct kfgx_cmd_struct *);

static inline int check_option(const opt_t *opt)
{
    if (!opt) {
        pr_warn("option pointer is NULL");
        return -1;
    }

    if ((!opt->l_opt || !*opt->l_opt) && (!opt->s_opt || !*opt->s_opt)) {
        pr_warn("invalid option: must specify at least long_opt or short_opt");
        return -1;
    }

    return 0;
}

[[maybe_unused]]
static inline int set_token_dirty(token_t *t)
{
    if (!t)
        return -1;
    t->free = TOKEN_DIRTY;
    return 0;
}

static inline int set_token_valid(token_t *t)
{
    if (!t)
        return -1;
    t->free = TOKEN_VALID;
    return 0;
}

/**
 * @brief Free a CLI token list and its associated options.
 *
 * @param[in,out] t Head of the token linked list to release.
 *
 * @details Traverses the singly linked list starting at @p t. If the @c free
 *          flag of a token node is set, all allocated option fields and th
 *          option structure itself are released. Finally, frees each token
 * node.
 *
 * @note Safe to call with a NULL pointer.
 */
static inline void kfgx_token_free(token_t **t_ptr)
{
    token_t *next;
    token_t *t;

    if (!t_ptr || !*t_ptr) {
        pr_debug("token pointer is NULL");
        return;
    }

    t = *t_ptr;
    while (t) {
        next = t->next;
        if (t->free && t->opt) {
            free(t->opt->l_opt);
            free(t->opt->s_opt);
            free(t->opt->value);
            free(t->opt);
        }

        free(t);
        t = next;
    }

    /* Set caller's pointer to NULL */
    *t_ptr = NULL;
}

static inline token_t *kfgx_get_new_token(const opt_t *opt)
{
    token_t *t;

    if (check_option(opt)) {
        pr_warn("invalid option");
        return NULL;
    }

    t = calloc(1, sizeof(token_t));
    if (!t) {
        return NULL;
    }

    t->opt = malloc(sizeof(opt_t));
    if (!t->opt) {
        free(t);
        return NULL;
    }

    t->opt->l_opt = opt->l_opt ? strdup(opt->l_opt) : NULL;
    t->opt->s_opt = opt->s_opt ? strdup(opt->s_opt) : NULL;
    t->opt->value = opt->value ? strdup(opt->value) : NULL;

    t->next = NULL;
    t->free = 1;
    set_token_valid(t);

    return t;
}

// heat
static inline token_t *kfgx_get_new_token(const opt_t *);

/**
 * @brief Add a new option token to a registered handler.
 *
 * Allocates a new token wrapping the provided option, initializes its flags,
 * and prepends it to the handler's token linked list.
 *
 * @param[in,out] h   Pointer to the target CLI handler.
 * @param[in]     opt Pointer to the option structure to register.
 *
 * @return 0 on success.
 * @return -1 if @p h is not registered, @p opt is NULL, or allocation fails.
 */
static inline int add_new_option(handler_t *h, const opt_t *opt)
{
    token_t *t;

    if (check_handler(h) != HANDLER_REGISTERED || !opt) {
        return -1;
    }

    t = kfgx_get_new_token(opt);
    if (!t) {
        pr_warn("failed to allocate new token");
        return -1;
    }

    t->match = 0;
    t->free = 1;

    if (t->opt) {
        t->opt->value = NULL;
    }

    t->next = h->ltokens;
    h->ltokens = t;

    return 0;
}

/**
 * @brief Validate CLI arguments.
 *
 * @param[out] cmd       Parsed command structure.
 * @param[in]  args_nr   Expected number of arguments.
 * @param[in]  args_set  NULL-terminated argument array.
 *
 * @details In STRICT_MODE, also verifies that @p args_nr matches
 * the exact number of entries in @p args_set.
 *
 * @return 0 on success, -1 on error.
 */
static inline int check_cli_args(struct kfgx_cmd_struct *cmd)
{
    char **args_set = cmd->args_set;
    int nr [[maybe_unused]];
    char **set [[maybe_unused]];

    if (!cmd) {
        pr_warn("kfgx_cmd_struct *cmd=%p", (void *)cmd);
        return -1;
    }

    if (!args_set) {
        pr_warn("args_set=NULL, but args_nr=%d", cmd->args_nr);
        return -1;
    }

#ifdef STRICT_MODE
    nr = cmd->args_nr;
    set = args_set;

    while (*set++) {
        --nr;
    }

    if (nr != 0) {
        pr_fatal("args_set element count != args_nr, remaining=%d", nr);
        exit(-1);
    }
#endif
    return 0;
}

#endif /* INCLUDE_CLICNTL_H */