#ifndef MANAKNIGHT_FORMATTER_H
#define MANAKNIGHT_FORMATTER_H

#include "ast.h"
#include <stdio.h>

// Formatter
typedef struct {
    char* buffer;
    size_t buffer_size;
    size_t buffer_capacity;
    int indent_level;
} Formatter;

Formatter* formatter_create(void);
void formatter_free(Formatter* formatter);
void formatter_format_program(Formatter* formatter, Program* program);
char* formatter_get_code(Formatter* formatter);

#endif // MANAKNIGHT_FORMATTER_H
