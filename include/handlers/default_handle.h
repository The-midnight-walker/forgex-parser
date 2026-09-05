// SPDX-License-Identifier: GPL-2.0
//
// vim: set ts=8 sw=8 noet tw=80 cc=80 fo+=t :

/**
 * @file      default_handle.h
 * @author    midnight walker
 * @brief     Declaration of the internal fallback CLI handler.
 * @version   0.1
 * @date      2026-09-05
 *
 * @details   Exposes the default fallback handler used when no explicit handler
 *            matches the provided command-line arguments.
 *
 * @copyright GNU General Public License v2.0
 */

#ifndef INCLUDE_DEFAULT_HANDLE_H
#define INCLUDE_DEFAULT_HANDLE_H

#include "handlers.h"

/**
 * @brief Internal fallback handler registered during initialization.
 */
extern handler_t kfgx_default_handler;

#endif /*INCLUDE_DEFAULT_HANDLE_H*/