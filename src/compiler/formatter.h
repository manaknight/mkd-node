#ifndef FORMATTER_H
#define FORMATTER_H

#include "ast.h"
#include <stdio.h>

// Formatter structure
typedef struct {
    FILE* output;
    int indent_level;
    bool needs_indent;
} Formatter;

// Function declarations
Formatter* create_formatter(FILE* output);
void free_formatter(Formatter* formatter);

// Main formatting functions
void format_program(Formatter* formatter, Program* program);
void format_module(Formatter* formatter, Module* module);
void format_api(Formatter* formatter, ApiRoute* api);
void format_declaration(Formatter* formatter, Decl* decl);
void format_statement(Formatter* formatter, Stmt* stmt);
void format_expression(Formatter* formatter, Expr* expr);
void format_type(Formatter* formatter, Type* type);
void format_pattern(Formatter* formatter, Pattern* pattern);

// Helper functions
void indent(Formatter* formatter);
void newline(Formatter* formatter);
void write_string(Formatter* formatter, const char* str);
void write_indent(Formatter* formatter);

#endif // FORMATTER_H
