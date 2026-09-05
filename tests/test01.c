// SPDX-License-Identifier: GPL-2.0
//
// vim: set ts=8 sw=8 noet tw=80 cc=80 fo+=t :

#include "clicntl.h"
#include "parser.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int dummy_action(opt_t *opt)
{
    (void)opt;
    return 0;
}

static int init_test_options(handler_t *h)
{
    add_new_option(h, &(opt_t){.l_opt = "--verbose", .s_opt = "-v"});
    add_new_option(h, &(opt_t){.l_opt = "--help", .s_opt = "-h"});
    add_new_option(h, &(opt_t){.l_opt = "--level", .s_opt = "-l"});
    add_new_option(h, &(opt_t){.s_opt = "-q"}); // short only (no l_opt)
    return 0;
}

static void test_matching(void)
{
    const char *dummy_argv[] = {"test_prog", NULL};
    init_handling(1, dummy_argv);

    handler_t h = {
        .name = "test",
        .prio = 1,
        .action = dummy_action,
        .init_opt = init_test_options,
    };

    int reg = register_handler(&h);
    assert(reg == 0);

    h.init_opt(&h);

    char *argv[] = {"--verbose", "-h", "--level=42", NULL};
    struct cmd_struct cmd = {
        .handler = &h,
        .args_nr = 3,
        .args_set = argv,
    };

    int ret = kfgx_cli_parser(&cmd);
    assert(ret == 0);

    // Verify matched tokens
    token_t *t = h.ltokens;
    assert(t != NULL);

    int count = 0;
    while (t) {
        if (t->opt) {
            count++;
            if (t->opt->l_opt && strcmp(t->opt->l_opt, "--level") == 0) {
                assert(
                    t->opt->value != NULL && strcmp(t->opt->value, "42") == 0);
            }
        }
        t = t->next;
    }
    assert(count == 3);

    kfgx_token_free(&h.ltokens);
    unregister_handler(&h);
    printf("[PASS] test_matching\n");
}

static void test_unrecognized_option(void)
{
    const char *dummy_argv[] = {"test_prog", NULL};
    init_handling(1, dummy_argv);

    handler_t h = {
        .name = "test_unrec",
        .prio = 1,
        .action = dummy_action,
        .init_opt = init_test_options,
    };

    int reg = register_handler(&h);
    assert(reg == 0);

    h.init_opt(&h);

    char *argv[] = {"--unrecognized", NULL};
    struct cmd_struct cmd = {
        .handler = &h,
        .args_nr = 1,
        .args_set = argv,
    };

    int ret = kfgx_cli_parser(&cmd);
    assert(ret == -1);
    assert(h.ltokens == NULL);

    unregister_handler(&h);
    printf("[PASS] test_unrecognized_option\n");
}

static void test_short_only_option(void)
{
    const char *dummy_argv[] = {"test_prog", NULL};
    init_handling(1, dummy_argv);

    handler_t h = {
        .name = "test_short",
        .prio = 1,
        .action = dummy_action,
        .init_opt = init_test_options,
    };

    int reg = register_handler(&h);
    assert(reg == 0);

    h.init_opt(&h);

    char *argv[] = {"-q", NULL};
    struct cmd_struct cmd = {
        .handler = &h,
        .args_nr = 1,
        .args_set = argv,
    };

    int ret = kfgx_cli_parser(&cmd);
    assert(ret == 0);
    assert(h.ltokens != NULL);
    assert(h.ltokens->opt != NULL);
    assert(strcmp(h.ltokens->opt->s_opt, "-q") == 0);

    kfgx_token_free(&h.ltokens);
    unregister_handler(&h);
    printf("[PASS] test_short_only_option\n");
}

int main(void)
{
    printf("=== Running Kernforgex Test Suite ===\n");
    test_matching();
    test_unrecognized_option();
    test_short_only_option();
    printf("All tests passed successfully!\n");
    return 0;
}
