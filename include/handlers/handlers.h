// SPDX-License-Identifier: GPL-2.0
//
// vim: set ts=8 sw=8 noet tw=80 cc=80 fo+=t :

#ifndef INCLUDE_HANDLERS_H
#define INCLUDE_HANDLERS_H

#define HANDLER_MIN_PRIO -10
#define HANDLER_MAX_PRIO 20

enum handler_state {
    HANDLER_UNREGISTERED = -3,
    HANDLER_BAD_CONF = -2,
    HANDLER_UNKNOWN = -1,
    HANDLER_REGISTERED = 0,
};

struct cmd_struct;
typedef struct kfgx_token token_t;
typedef struct handler handler_t;
typedef struct kfgx_opt opt_t;
typedef int (*init_options_t)(handler_t *);
typedef int (*handler_action_t)(opt_t *opt);

/**
 * @brief Action callback executed when a handler matches the current CLI input.
 *
 * @param[in,out] opt Pointer to the matched option list for the command.
 * @return 0 on success, or a negative error code on failure.
 */
typedef struct handler {
    handler_action_t action;
    init_options_t init_opt;
    const char *name;
    const int prio;
    token_t *ltokens; /* head (last add to the first) of Linked list of
                         associated options */
    struct handler *next;
} handler_t;

/**
 * @brief Sets the global default handler for CLI execution.
 *
 * Configures the fallback handler to be used when no explicit handler
 * matches the provided command line arguments or when no arguments are given.
 *
 * @param[in] a Pointer to the handler structure to set as default.
 *
 * @return 0 on success.
 * @return -1 if @p a is NULL or fails handler validation checks.
 *
 * @note The handler passed to this function should be properly initialized
 *       before being set as default.
 */
int set_default_handler(const handler_t *a);

/**
 * @brief Main CLI dispatch and handling function.
 *
 * Resolves the matching handler from the registered priority list, tokenizes
 * input arguments, invokes the handler action callback, and cleans up memory.
 *
 * @return 0 on successful processing and execution.
 * @return -1 on argument validation error, unhandled command, or parsing error.
 */
int handle(void);

/**
 * @brief Validates a handler structure and checks its registration state.
 *
 * Verifies that the handler has a valid name string, a non-NULL action
 * callback, and a priority within [HANDLER_MIN_PRIO, HANDLER_MAX_PRIO]. Then
 * checks if it is already present in the registered handlers list.
 *
 * @param[in] h Pointer to the handler to validate.
 *
 * @return HANDLER_REGISTERED (0) if the handler is already registered.
 * @return HANDLER_UNREGISTERED (-3) if the handler is valid but not registered.
 * @return HANDLER_BAD_CONF (-2) if validation checks fail.
 * @return -1 on internal context failure.
 */
int check_handler(const handler_t *h);

/**
 * @brief Registers a new task handler into the global priority list.
 *
 * Validates that the handler is unregistered and correctly configured, then
 * inserts it into the priority-ordered list.
 *
 * @param[in] h Pointer to the handler to register.
 *
 * @return 0 on success, -1 on failure.
 */
int register_handler(handler_t *h);

/**
 * @brief Unregisters and removes a specific task handler from the global list.
 *
 * @param[in] h Pointer to the handler to unregister.
 *
 * @return 0 on success, -1 on failure.
 */
int unregister_handler(handler_t *h);

/**
 * @brief Initializes the CLI execution context with user arguments.
 *
 * Sets up the internal priority list, registers the internal fallback handler,
 * and initializes command structure with the argument vector (excluding the
 * program name).
 *
 * @param[in] argc Number of command-line arguments.
 * @param[in] argv Array of argument strings.
 *
 * @return 0 on success, -1 on initialization failure.
 */
int init_handling(const int argc, const char **argv);

/**
 * @brief Generates POSIX sh-compatible auto-completion scripts for registered
 * handlers.
 *
 * Iterates through registered handlers and generates completion script entries
 * in completions.sh.
 *
 * @return 0 on success, -1 on failure.
 */
int generate_sh_completions(void);

/**
 * @brief Generates Bash auto-completion scripts for registered handlers.
 *
 * Initializes completions.bash and writes completion functions and complete -F
 * directives for each registered handler with options.
 *
 * @return 0 on success, -1 on failure.
 */
int generate_bash_completions(void);

#endif /*INCLUDE_HANDLERS_H*/