# kernforgex

This project is not the final tool itself. It is the small infrastructure layer that was created while building `kernforgex` another repository on my github. At its core, it was first and foremost a parser and command-dispatch framework for Linux-oriented tooling.

## What this project is

This repository is a lightweight C framework for creating Linux command-line tools quickly.

Its purpose is to provide the base for:
- argument parsing,
- handler dispatch,
- shell completion generation,

The goal is not to be a finished end-user utility by itself, but to be the infrastructure that enables building one.

## Why it exists

When working on a bigger tool, the first thing you need is not the final feature set — it is the mechanism that makes commands possible.

Before the real command exists, you need:
- a CLI parser,
- a simple option model,
- a way to wire commands to handlers,
- a central entry point,
- reusable patterns for help/version/verbose mode,
- shell completion support.

This project is exactly that foundation.

## Core execution pattern

The main execution flow is defined in `src/main.c`.

```c

#include <stdio.h>
#include <string.h>

#define STRICT_MODE

// include handlers header
#include "clicntl.h"

// include our header
#include "my_handler.h"

int main(int argc, char *argv[])
{

    // init command line handling
    init_handling(argc,argv);

    // init our handler
    // insert our default handler in our case it is under src/handlers
    if (my_handler_init()) {
        return -1;
    }

    // avoid this cause high memory on free area
    generate_bash_completions();

    // handling
    if (handle()) {
        fprintf(stderr, "Error: Parsing failed\n");
        return -1;
    }

    return 0;
}
```

This is the canonical example to follow when creating a new command-line tool from this base.

The logic is simple:
1. initialize CLI handling,
2. register the custom handler,
4. execute the matching command,
5. return the result.

## The reference handler example

The file `src/handlers/my_handler.c` is the prototype example to follow.

It defines the actual command behavior, the registered options, and the handler object used by the dispatcher.

### Option registration

```c
static int my_init_options(handler_t *h)
{
    add_new_option(
        h,
        &(opt_t){
            .l_opt = "--help", .s_opt = "-h"});
    add_new_option(
        h,
        &(opt_t){
            .l_opt = "--version", .s_opt = "-v"});
    add_new_option(
        h,
        &(opt_t){
            .l_opt = "--verbose", .s_opt = "-t"});

    return 0;
}
```

### Action callback

```c
static int my_handler_action(opt_t *options)
{
    opt_t *o;

    if(!options){
        my_handler_usage();
    }else{
        foreach_node(o, options){
            if(HAVE_OPTION(o->l_opt, "--help"))
                my_handler_usage();

            if(HAVE_OPTION(o->l_opt, "--verbose"))
                printf("we are on verbose mode\n");

            if(HAVE_OPTION(o->l_opt, "--version"))
                printf("we are on version 1.0.0\n");
        }
    }
    return 0;
}
```

### Handler definition

```c
static handler_t my_handler = {
    .action = my_handler_action,
    .init_opt = my_init_options,
    .name = "default",
    .prio = -1,
    .ltokens = NULL,
    .next = NULL,
};
```

This is the exact model to follow when creating your own command family.

## Useful APIs in this project

The most relevant APIs available in the current codebase are:

- `init_handling(int argc, char **argv)`
  - prepares the CLI environment
- `register_handler(handler_t *h)`
  - registers a new command handler
- `set_default_handler(handler_t *h)`
  - assigns the fallback handler
- `handle()`
  - dispatches to the correct command handler
- `add_new_option(handler_t *h, const opt_t *opt)`
  - registers a new option
- `generate_bash_completions()`
  - generates Bash completion metadata
- `generate_sh_completions()`
  - generates shell completion metadata
- `check_handler(const handler_t *)`
  - validates handler integrity

These are the main building blocks for building the real command later.

## Completion generation

This project includes shell completion generation helpers:

- `generate_bash_completions()`
- `generate_sh_completions()`

This part is useful for ergonomic shell integration, but in the current version it is still a bit heavy.

### Important note

At the present stage, the completion-generation path can consume more memory than the normal runtime flow. Because of that, a common workflow during development is:

- build once with completion generation enabled,
- build another lightweight version without it for optimized runtime usage.

This is not a final design decision; it is a temporary constraint while the completion system is being refined.

The long-term goal is to make shell completion optional, lighter, and cleaner.

## Build

```bash
cmake -S . -B build
cmake --build build
```
