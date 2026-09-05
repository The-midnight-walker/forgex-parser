# SPDX-License-Identifier: GPL-2.0
#
# vim: set ts=8 sw=8 noet tw=80 cc=80 fo+=t :

file(GLOB_RECURSE SOURCE_FILES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/*.c"
)

# executable name : kfgx
add_executable(${PROJECT_NAME} ${SOURCE_FILES})

target_compile_features(${PROJECT_NAME} PRIVATE
    c_std_11
)

target_compile_definitions(${PROJECT_NAME} PRIVATE
    _POSIX_C_SOURCE=200809L
    DEBUG
    PROJECT_NAME="${PROJECT_NAME}"
)

target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_SOURCE_DIR}/include/
    ${CMAKE_SOURCE_DIR}/include/cli/
    ${CMAKE_SOURCE_DIR}/include/handlers/
    ${CMAKE_SOURCE_DIR}/src/handlers/
)

target_compile_options(${PROJECT_NAME} PRIVATE
    -Wall
    -Wextra
    -Wshadow
    -Werror
    -Wconversion
    -Wformat=2
    -fmacro-prefix-map=${CMAKE_SOURCE_DIR}=.
)