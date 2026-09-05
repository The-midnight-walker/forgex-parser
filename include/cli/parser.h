// SPDX-License-Identifier: GPL-2.0
//
// vim: set ts=8 sw=8 noet tw=80 cc=80 fo+=t :

/**
 * @file      parser.h
 * @author    midnight walker
 * @brief     Public interface for the kernforgex CLI argument parser and
 * tokenizer.
 * @version   0.2
 * @date      2026-09-05
 *
 * @details   Exposes core parsing functions to tokenize command line arguments,
 *            match long and short options, handle key=value assignments, and
 * chain matched options for the active command handler.
 *
 * @copyright GNU General Public License v2.0
 */

#ifndef INCLUDE_PARSER_H
#define INCLUDE_PARSER_H

#include "clicntl.h"

/**
 * @brief Tokenizes and validates CLI arguments against the registered handler
 * options.
 *
 * Validates the command structure, iterates over all CLI arguments, matches
 * both short and long options, processes key-value assignments, and prunes
 * unmatched nodes.
 *
 * @param[in,out] cmd Pointer to the command context structure containing
 * argument vector and the active handler's token list.
 *
 * @return 0 on success (all arguments valid and matched).
 * @return -1 on parsing error, unrecognized option, or invalid input structure.
 */
int kfgx_cli_tokenizer(struct cmd_struct *cmd);

/**
 * @brief High-level entry point to parse command-line arguments for a command.
 *
 * Validates handler presence and dispatches argument parsing to the tokenizer.
 *
 * @param[in,out] cmd Pointer to the command context structure to parse.
 *
 * @return 0 on successful parsing.
 * @return -1 if handler is missing or parsing fails.
 */
int kfgx_cli_parser(struct cmd_struct *cmd);

#endif /* INCLUDE_PARSER_H */
