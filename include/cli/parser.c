// SPDX-License-Identifier: GPL-2.0
//
// vim: set ts=8 sw=8 noet tw=80 cc=80 fo+=t :

/**
 * @file      parser.c
 * @author    midnight walker
 * @brief     Parse and tokenize CLI arguments for registered handlers.
 * @version   0.1
 * @date      2026-08-29
 *
 * @details   The tokenizer validates long and short options, handles
 *            key=value assignments, removes unmatched entries, and prepares
 *            a linked list of matched options for the active handler.
 *
 * @copyright GNU General Public License v2.0
 */

#define prfx_fmt "parser: "

#include "clicntl.h"
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#define OPT_MATCH 0     // option match
#define OPT_NOT_MATCH 1 // option not match

/**
 * @brief Prunes unmatched tokens from the chain and frees their allocated
 * memory in-place.
 *
 * @param[in,out] head Double pointer to the head of the token list.
 */
static void kfgx_tokens_cleanup_unmatched(token_t **head)
{
    token_t *curr;
    token_t *prev = NULL;

    if (!head || !*head) {
        return;
    }

    curr = *head;

    while (curr) {
        token_t *next = curr->next;

        if (!curr->match) {
            /* Unlink current unmatched node */
            if (prev) {
                prev->next = next;
            } else {
                *head = next;
            }

            curr->next = NULL;
            /* Safely free node using double pointer interface */
            kfgx_token_free(&curr);

            curr = next;
        } else {
            prev = curr;
            curr = next;
        }
    }

    pr_debug("Cleaned up unmatched options, new head=%p", (void *)*head);
}

/**
 * @brief Chains the option structures of sequentially matched tokens.
 *
 * Traverses the filtered token list and links each option's @c next pointer
 * to the subsequent token's option structure. Ensures the last matched option's
 * @c next pointer is explicitly set to NULL.
 *
 * @param[in,out] head Head of the (filtered) matched tokens list.
 *
 * @return 0 on success.
 * @return -1 if @p head is NULL or an invalid token containing a NULL option is
 * found.
 */
[[maybe_unused]]
static void chained_matched_options(token_t *head)
{
    token_t *t;

    if (!head)
        pr_debug("token list is empty");

    foreach_node(t, head)
    {
        if (!t) {
            pr_debug("no option found to chained");
            return;
        }

        if (!t->opt) {
            pr_fatal("NULL option found: cleanup or token setup failed");
            exit(-1);
        }

        if (t->next && t->next->opt) {
            t->opt->next = t->next->opt;
        } else {
            t->opt->next = NULL;
        }
    }
    pr_debug("chained matched options");
}

// |------------------ Tokenizer core
static int has_match_l_opt(token_t *head, const char *arg)
{
    token_t *t;

    if (!head || !arg) {
        pr_error("tokens=%p, arg=%p", (void *)head, (void *)arg);
        return -1;
    }

    foreach_node(t, head)
    {
        if (t->opt && HAVE_OPTION(t->opt->l_opt, arg)) {
            t->match = 1;
            return OPT_MATCH;
        }
    }
    return OPT_NOT_MATCH;
}

static int has_match_s_opt(token_t *head, const char *arg)
{
    token_t *t;

    if (!head || !arg) {
        pr_error("tokens=%p, arg=%p", (void *)head, (void *)arg);
        return -1;
    }

    /* Validate short option syntax: "-a" or grouped "-abc" (excluding "--") */
    if (arg[0] == '-' && arg[1] != '\0' && arg[1] != '-') {
        for (const char *p = arg + 1; *p != '\0'; p++) {
            char short_str[2] = {*p, '\0'};
            foreach_node(t, head)
            {
                /* Compare while skipping the leading '-' in opt->s_opt */
                if (t->opt && t->opt->s_opt && t->opt->s_opt[0] != '\0' &&
                    HAVE_OPTION(t->opt->s_opt + 1, short_str)) {
                    t->match = 1;
                    goto next;
                }
            }
            return OPT_NOT_MATCH;
        next:
        }
    }

    return OPT_MATCH;
}

static int has_value(token_t *head, const char *arg)
{
    size_t key_len;
    token_t *t;
    char *key = NULL;
    char *c = NULL;
    int ret = OPT_NOT_MATCH;

    if (!head || !arg) {
        pr_error("tokens=%p, arg=%p", (void *)head, (void *)arg);
        return -1;
    }

    /* Locate the first '=' delimiter */
    c = strchr(arg, '=');
    if (!c) {
        pr_debug("no assign with '=' found");
        return has_match_l_opt(head, arg);
    }

    /* Ensure a value exists after '=' */
    if (*(c + 1) == '\0') {
        pr_debug("no value assigned after '='");
        return ret;
    }

    /* Extract key substring prior to '=' */
    key_len = (size_t)(c - arg);
    key = malloc(key_len + 1);
    if (!key) {
        pr_error("failed to allocate memory for 'key'");
        return -1;
    }

    memcpy(key, arg, key_len);
    key[key_len] = '\0';

    /* Match key against registered long options */
    foreach_node(t, head)
    {
        if (t->opt && HAVE_OPTION(t->opt->l_opt, key)) {
            t->match = 1;

            /* Prevent memory leaks if option value is reassigned */
            if (t->opt->value) {
                free(t->opt->value);
            }

            t->opt->value = strdup(c + 1);
            pr_debug("Matched option: %s with value: %s", key, t->opt->value);
            ret = OPT_MATCH;
            goto end;
        }
    }

end:
    free(key);
    return ret;
}

/**
 * @brief Parses CLI arguments and validates matching tokens.
 *
 * @param[in,out] cmd Structure containing argument vector and handler tokens.
 * @return 0 on success, -1 on parsing error or invalid input structure.
 */
static int kfgx_cli_tokenizer_impl(struct cmd_struct *cmd)
{
    char **set;
    token_t *head;
    int ret;

    if (!cmd || !cmd->handler || !cmd->handler->ltokens || !cmd->args_set) {
        pr_warn("missing some fields from the user command line");
        return -1;
    }

    set = cmd->args_set;
    head = cmd->handler->ltokens;

    while (*set) {
        const char *arg = *set;

        /* Check for key=value assignment syntax */
        if (strchr(arg, '=')) {
            ret = has_value(head, arg);
            if (ret != OPT_MATCH) {
                goto err;
            } else if (ret == -1) { // internal program error
                return -1;
            }
            goto next;
        } else {
            /* Try matching long option first (e.g., --help) */
            ret = has_match_l_opt(head, arg);
            if (ret == OPT_MATCH) {
                goto next;
            } else if (ret == -1) { // internal program error
                return -1;
            }

            /* Fallback to short option matching (e.g., -h or -vh) */
            ret = has_match_s_opt(head, arg);
            if (ret == OPT_NOT_MATCH) {
                /* Unrecognized command-line argument */
                pr_error("Unrecognized option: %s", arg);
                goto err;
            } else if (ret == -1) { // internal program error
                return -1;
            }
            goto next;
        }
    err:
        kfgx_token_free(&cmd->handler->ltokens);
        return 0;

    next:
        set++;
    }
    /* Prune unmatched nodes and link active options chain */
    kfgx_tokens_cleanup_unmatched(&cmd->handler->ltokens);
    chained_matched_options(cmd->handler->ltokens);

    return 0;
}

/**
 * @brief Tokenize CLI arguments.
 *
 * @param[out] cmd       Parsed command structure.
 *
 * @return 0 on success, -1 on error.
 */
int kfgx_cli_tokenizer(struct cmd_struct *cmd)
{
    if (check_cli_args(cmd))
        return -1;

    return kfgx_cli_tokenizer_impl(cmd);
}

static int kfgx_cli_parser_impl(struct cmd_struct *cmd)
{
    return kfgx_cli_tokenizer_impl(cmd);
}

/**
 * @brief Parse command-line arguments into a token list.
 *
 * @param[out] cmd   Parsed command structure.
 *
 * @return 0 on success, -1 on error.
 */
int kfgx_cli_parser(struct cmd_struct *cmd)
{
    if (!cmd->handler) {
        pr_warn("handler=%p", (void *)cmd->handler);
        return -1;
    }

    return kfgx_cli_parser_impl(cmd);
}