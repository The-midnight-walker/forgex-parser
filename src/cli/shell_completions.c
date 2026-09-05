// SPDX-License-Identifier: GPL-2.0
//
// vim: set ts=8 sw=8 noet tw=80 cc=80 fo+=t :

/**
 * @file      shell_completions.c
 * @author    midnight walker
 * @brief     Generate Bash and sh completion scripts for registered handlers.
 * @version   0.1
 * @date      2026-09-01
 *
 * @details   The file produces completion metadata for the CLI options of a
 *            handler so the shell can offer context-aware suggestions.
 *
 * @copyright GNU General Public License v2.0
 */
#include "clicntl.h"

static int sh_completions_impl(const handler_t *h, const char *filename)
{
    token_t *t;
    FILE *f;

    f = fopen(filename, "a");
    if (f == NULL) {
        pr_error("cannot open file %s: %s", filename, strerror(errno));
        return -1;
    }

    fprintf(
        f,
        "_%s_completion()\n"
        "{\n"
        "    local cur=\"${COMP_WORDS[COMP_CWORD]}\"\n\n"
        "    COMPREPLY=(\n"
        "        $(compgen -W \"", // auto add
        h->name);

    if (h->ltokens) {
        foreach_node(t, h->ltokens)
        {
            if (!t || !t->opt)
                continue;

            if (t->opt->l_opt && t->opt->s_opt) {
                fprintf(f, "%s %s ", t->opt->l_opt, t->opt->s_opt);
            } else if (t->opt->l_opt) {
                fprintf(f, "%s ", t->opt->l_opt);
            } else if (t->opt->s_opt) {
                fprintf(f, "%s ", t->opt->s_opt);
            }
        }
    }

    fprintf(
        f,
        "\" -- \"$cur\"))\n"
        "}\n\n"
        "complete -F _%s_completion %s\n",
        h->name,
#ifdef PROJECT_NAME
        PROJECT_NAME
#else
        h->name
#endif
    );

    fclose(f);
    pr_info(
        "generated bash completions file for handler '%s' in '%s'",
        h->name,
        filename);

    return 0;
}

/**
 * @brief Generates a Bash auto-completion script for a given CLI handler.
 *
 * Iterates through the option list associated with the specified command
 * handler and constructs a formatted sh under posixcompletion script file.
 *
 * @param[in] h        Pointer to the command handler configuration structure.
 * @param[in] filename Output file path where the completion script will be
 * written.
 *
 * @pre \p h and \p filename must not be NULL.
 * @pre \p h->name must be a valid null-terminated string.
 *
 * @return int Operation status code:
 * @retval 0  Success. Script generated without errors.
 * @retval -1 Error. Invalid input parameters or file I/O failure.
 */
int bash_completions(handler_t *h)
{
    if (!h || !h->name) {
        return -1;
    }

    if (!h->ltokens || !h->ltokens->opt) {
        pr_debug("handler %s has no options", h->name);
    }

    return sh_completions_impl(h, BASH_FILENAME);
}

/**
 * @brief Generates a sh auto-completion script for a given CLI handler.
 *
 * Iterates through the option list associated with the specified command
 * handler and constructs a formatted sh under posixcompletion script file.
 *
 * @param[in] h        Pointer to the command handler configuration structure.
 *
 * @pre \p h->name must be a valid null-terminated string.
 *
 * @return int Operation status code:
 * @retval 0  Success. Script generated without errors.
 * @retval -1 Error. Invalid input parameters or file I/O failure.
 */
int sh_completions(handler_t *h)
{
    if (!h || !h->name) {
        return -1;
    }

    if (!h->ltokens || !h->ltokens->opt) {
        pr_debug("handler %s has no options", h->name);
    }

    return sh_completions_impl(h, SH_FILENAME);
}