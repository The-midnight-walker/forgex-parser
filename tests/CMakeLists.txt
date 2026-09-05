# SPDX-License-Identifier: GPL-2.0
#
# vim: set ts=8 sw=8 noet tw=80 cc=80 fo+=t :

add_executable(test01
    test01.c
    ${CMAKE_SOURCE_DIR}/src/cli/parser.c
    ${CMAKE_SOURCE_DIR}/src/cli/shell_completions.c
    ${CMAKE_SOURCE_DIR}/src/handlers/handle.c
    ${CMAKE_SOURCE_DIR}/src/handlers/default_handle.c
    ${CMAKE_SOURCE_DIR}/src/debug.c
)

target_include_directories(test01 PRIVATE
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/include/cli
    ${CMAKE_SOURCE_DIR}/include/handlers
)

target_compile_features(test01 PRIVATE c_std_11)
target_compile_definitions(test01 PRIVATE _POSIX_C_SOURCE=200809L DEBUG)
target_compile_options(test01 PRIVATE -Wall -Wextra -Werror)

add_test(NAME test01 COMMAND test01)
