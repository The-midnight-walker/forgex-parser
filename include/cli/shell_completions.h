// SPDX-License-Identifier: GPL-2.0
//
// vim: set ts=8 sw=8 noet tw=80 cc=80 fo+=t :

#ifndef INCLUDE_SHELL_COMPLETIONS_H
#define INCLUDE_SHELL_COMPLETIONS_H

#define BASH_FILENAME "completions.bash"
#define SH_FILENAME "completions.sh"

/**
 * @brief Initializes the Bash completion file with the standard shebang header.
 *
 * This function creates or truncates the file defined by \ref BASH_FILENAME
 * and writes the initial executable script line (`#!/usr/bin/env bash`).
 *
 * @note The file stream is closed upon function completion.
 *
 * @pre The macro \ref BASH_FILENAME must contain a valid file system path.
 *
 * @return int Operation status code:
 * @retval 0  Success. The completion file was initialized properly.
 * @retval -1 Error. Failed to open the target file (check logs or \c errno).
 */
static inline int init_bash_completions_file(void)
{
    FILE *file = fopen(BASH_FILENAME, "w");
    if (file == NULL) {
        pr_error("cannot open file %s: %s", BASH_FILENAME, strerror(errno));
        return -1;
    }

    fprintf(file, "%s\n\n", "#!/usr/bin/env bash");

    fclose(file);

    return 0;
}

/**
 * @brief Generates a POSIX-compliant script header for auto-completion.
 *
 * @note POSIX sh and dash do not natively support interactive completion
 * arrays. This generates a standard /bin/sh fallback header.
 *
 * @return int Operation status code:
 * @retval 0  Success. The completion file was initialized properly.
 * @retval -1 Error. Failed to open the target file (check logs or \c errno).
 */
static inline int init_sh_completions_file(void)
{
    FILE *file = fopen(SH_FILENAME, "w");
    if (file == NULL) {
        pr_error("cannot open file %s: %s", SH_FILENAME, strerror(errno));
        return -1;
    }

    /* POSIX compliant shebang */
    fprintf(file, "%s\n", "#!/bin/sh");

    fclose(file);
    return 0;
}

/**
 * @brief Generates Bash auto-completion script block for a given CLI handler.
 *
 * Appends a Bash completion function and complete directive to the Bash
 * completion file.
 *
 * @param[in] h Pointer to the command handler configuration structure.
 *
 * @return 0 on success, -1 on error.
 */
int bash_completions(handler_t *h);

/**
 * @brief Generates POSIX sh auto-completion script block for a given CLI
 * handler.
 *
 * Appends a POSIX-compliant completion function to the sh completion file.
 *
 * @param[in] h Pointer to the command handler configuration structure.
 *
 * @return 0 on success, -1 on error.
 */
int sh_completions(handler_t *h);

#endif /*INCLUDE_SHELL_COMPLETIONS_H*/