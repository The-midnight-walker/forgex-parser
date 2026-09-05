# forgex-parser

[![License: GPL v2](https://img.shields.io/badge/License-GPLv2-blue.svg)](LICENSE)
[![Standard: C11](https://img.shields.io/badge/Standard-C11-green.svg)](https://en.wikipedia.org/wiki/C11_(C_standard_revision))
[![Platform: Linux](https://img.shields.io/badge/Platform-Linux-orange.svg)](https://kernel.org)

**forgex-parser** is a lightweight, modular C framework for building extensible Linux command-line tools. It provides a priority-ordered command dispatcher, a flexible argument tokenizer, automatic shell completion generation, and kernel-style logging.

---

## Key Features

- **Priority-Ordered Handler Dispatcher**: Register commands with integer priorities; the dispatcher automatically matches CLI inputs or routes to configurable fallback handlers.
- **Robust Argument Tokenizer**: Supports long options (`--option`), short options (`-v`), clustered short options (`-vh`), and key-value assignments (`--level=42`).
- **Dynamic Shell Completion Generation**: Generates Bash (`completions.bash`) and POSIX sh scripts directly from registered handler definitions.
- **Kernel-Style Logging**: Color-coded, bitmask-filtered logging macros (`pr_info`, `pr_debug`, `pr_warn`, `pr_error`, `pr_fatal`).
- **Strict Quality Standards**: Clean compilation under `-Wall -Wextra -Wshadow -Werror -Wconversion -Wformat=2`.

---

## Project Layout

```text
forgex-parser/
├── CMakeLists.txt          # Root build configuration, formatting, doc & test targets
├── include/                # Public and core headers (API definitions only)
│   ├── cli/
│   │   ├── clicntl.h       # Master CLI structures, token macros, and helper functions
│   │   ├── parser.h        # Tokenizer and parser function prototypes
│   │   └── shell_completions.h # Shell completion script headers and generators
│   ├── handlers/
│   │   ├── default_handle.h# Fallback handler declaration
│   │   ├── handlers.h      # Handler registration and dispatch engine
│   │   └── prio_lists.h    # Intrusive priority list implementation
│   ├── debug.h             # Logging macros and level definitions
│   └── errors.h            # Error code definitions
├── src/                    # Implementation units (.c)
│   ├── CMakeLists.txt      # Source target build configuration
│   ├── main.c              # Application entry point
│   ├── debug.c             # Log-level bitmask management
│   ├── cli/
│   │   ├── parser.c        # Tokenizer and CLI parsing logic
│   │   └── shell_completions.c # Bash & sh completion script writers
│   └── handlers/
│       ├── handle.c        # Handler priority queue, dispatching, lifecycle
│       ├── default_handle.c# Default fallback handler implementation
│       ├── my_handler.c    # Reference example handler
│       └── my_handler.h    # Example handler header
└── tests/                  # Automated test suite
    ├── CMakeLists.txt      # Test target configuration (CTest)
    └── test01.c            # Parser, tokenization, and validation unit tests
```

---

## Quick Start

### Prerequisites

- **CMake** >= 3.10
- **C11** compatible compiler (GCC or Clang)
- *(Optional)* **clang-format** (for code formatting)
- *(Optional)* **Doxygen** (for generating HTML documentation)

### Build

```bash
# Configure build
cmake -B build

# Compile project
cmake --build build
```

The resulting executable will be located at `build/src/kfgx`.

### Running Tests

```bash
ctest --test-dir build --output-on-failure
```

### Code Formatting & Documentation

```bash
# Format code using clang-format
cmake --build build --target format

# Generate Doxygen documentation (if Doxygen is installed)
cmake --build build --target doc
```

---

## Architecture & How It Works

### 1. Central Dispatch Flow (`src/main.c`)

```c
#include "clicntl.h"
#include "my_handler.h"

int main(int argc, char *argv[])
{
    // Initialize CLI context and argument tracking
    init_handling(argc, (const char **)argv);

    // Register your custom handler(s)
    if (my_handler_init()) {
        return -1;
    }

    // Generate Bash auto-completion script
    generate_bash_completions();

    // Parse arguments and dispatch matching handler
    if (handle()) {
        fprintf(stderr, "Error: Parsing failed\n");
        return -1;
    }

    return 0;
}
```

### 2. Creating a Custom Handler (`src/handlers/my_handler.c`)

A handler bundles an **action callback**, an **options initialization callback**, a **unique name**, and a **priority** (lower numerical values equal higher priority):

```c
#include "clicntl.h"
#include "my_handler.h"

// Define options associated with this handler
static int my_init_options(handler_t *h)
{
    add_new_option(h, &(opt_t){.l_opt = "--help", .s_opt = "-h"});
    add_new_option(h, &(opt_t){.l_opt = "--version", .s_opt = "-v"});
    add_new_option(h, &(opt_t){.l_opt = "--verbose", .s_opt = "-V"});
    return 0;
}

// Define action executed when this handler matches
static int my_handler_action(opt_t *options)
{
    opt_t *o;

    if (!options) {
        printf("No options specified. Showing usage...\n");
        return 0;
    }

    foreach_node(o, options) {
        if (HAVE_OPTION(o->l_opt, "--help"))
            printf("Showing help message.\n");

        if (HAVE_OPTION(o->l_opt, "--verbose"))
            printf("Verbose mode enabled.\n");

        if (HAVE_OPTION(o->l_opt, "--version"))
            printf("Version 1.0.0\n");
    }

    return 0;
}

// Declare handler structure
static handler_t my_handler = {
    .action = my_handler_action,
    .init_opt = my_init_options,
    .name = "default",
    .prio = -1,
    .ltokens = NULL,
    .next = NULL,
};

// Registration entry point
int my_handler_init(void)
{
    if (register_handler(&my_handler))
        return -1;

    // Set as default fallback handler when no command argument is specified
    if (set_default_handler(&my_handler))
        return -1;

    return 0;
}
```

---

## Shell Completions

Running `generate_bash_completions()` inspects all registered handlers and writes a complete Bash completion script to `completions.bash`:

```bash
# Enable completion in your current shell
source completions.bash

# Test completion:
./build/src/kfgx --<TAB><TAB>
# Suggestions: --help  --verbose  --version
```

---

## Core API Reference

| Function | Header | Description |
| :--- | :--- | :--- |
| `init_handling(argc, argv)` | [`handlers.h`](include/handlers/handlers.h) | Initializes internal priority lists and CLI context with input arguments. |
| `register_handler(h)` | [`handlers.h`](include/handlers/handlers.h) | Inserts a configured `handler_t` into the priority-sorted execution list. |
| `set_default_handler(h)` | [`handlers.h`](include/handlers/handlers.h) | Assigns the fallback handler invoked when no command name matches. |
| `handle()` | [`handlers.h`](include/handlers/handlers.h) | Resolves the active handler, parses tokens, executes the action callback, and frees resources. |
| `add_new_option(h, opt)` | [`clicntl.h`](include/cli/clicntl.h) | Allocates and attaches an option specification (`--long`, `-s`) to a handler. |
| `generate_bash_completions()` | [`handlers.h`](include/handlers/handlers.h) | Generates `completions.bash` script for all registered handlers with options. |
| `generate_sh_completions()` | [`handlers.h`](include/handlers/handlers.h) | Generates POSIX `completions.sh` fallback script. |
| `check_handler(h)` | [`handlers.h`](include/handlers/handlers.h) | Validates handler structure and verifies registration state. |

---

## License

This project is licensed under the GNU General Public License v2.0 - see the [LICENSE](LICENSE) file for details.
