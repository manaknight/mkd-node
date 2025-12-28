#include "formatter.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define INITIAL_BUFFER_SIZE 1024
#define BUFFER_GROWTH_FACTOR 2
#define INDENT_SIZE 4

static void formatter_grow_buffer(Formatter* formatter, size_t needed) {
    while (formatter->buffer_capacity < needed) {
        formatter->buffer_capacity *= BUFFER_GROWTH_FACTOR;
    }
    formatter->buffer = realloc(formatter->buffer, formatter->buffer_capacity);
}

static void formatter_append(Formatter* formatter, const char* str) {
    size_t len = strlen(str);
    size_t needed = formatter->buffer_size + len + 1;

    if (needed > formatter->buffer_capacity) {
        formatter_grow_buffer(formatter, needed);
    }

    strcpy(formatter->buffer + formatter->buffer_size, str);
    formatter->buffer_size += len;
}

static void formatter_append_indent(Formatter* formatter) {
    for (int i = 0; i < formatter->indent_level * INDENT_SIZE; i++) {
        formatter_append(formatter, " ");
    }
}

static void formatter_format_literal(Formatter* formatter, Literal* literal) {
    switch (literal->kind) {
        case LIT_STRING: {
            formatter_append(formatter, "\"");
            formatter_append(formatter, literal->value.string_val);
            formatter_append(formatter, "\"");
            break;
        }
        case LIT_INT64: {
            char buf[32];
            sprintf(buf, "%lld", (long long)literal->value.int64_val);
            formatter_append(formatter, buf);
            break;
        }
        case LIT_BOOL: {
            formatter_append(formatter, literal->value.bool_val ? "true" : "false");
            break;
        }
        case LIT_UNIT:
            formatter_append(formatter, "unit");
            break;
    }
}

static void formatter_format_expr(Formatter* formatter, void* expr_node);

static void formatter_format_call(Formatter* formatter, CallExpr* call) {
    formatter_format_expr(formatter, call->function);
    formatter_append(formatter, "(");

    for (size_t i = 0; i < call->argument_count; i++) {
        if (i > 0) formatter_append(formatter, ", ");
        formatter_format_expr(formatter, call->arguments[i]);
    }

    formatter_append(formatter, ")");
}

static void formatter_format_expr(Formatter* formatter, void* expr_node) {
    AstNode* node = (AstNode*)expr_node;

    switch (node->type) {
        case NODE_LITERAL:
            formatter_format_literal(formatter, (Literal*)expr_node);
            break;
        case NODE_IDENTIFIER_EXPR:
            formatter_append(formatter, ((IdentifierExpr*)expr_node)->name);
            break;
        case NODE_CALL_EXPR:
            formatter_format_call(formatter, (CallExpr*)expr_node);
            break;
        // TODO: Add more expression types
        default:
            formatter_append(formatter, "/* TODO: unimplemented expr */");
            break;
    }
}

static void formatter_format_block(Formatter* formatter, Block* block) {
    formatter_append(formatter, " {\n");
    formatter->indent_level++;

    for (size_t i = 0; i < block->statement_count; i++) {
        formatter_append_indent(formatter);
        // TODO: Handle different statement types
        formatter_append(formatter, "// TODO: statement\n");
    }

    if (block->result_expr) {
        formatter_append_indent(formatter);
        formatter_format_expr(formatter, block->result_expr);
        formatter_append(formatter, "\n");
    }

    formatter->indent_level--;
    formatter_append_indent(formatter);
    formatter_append(formatter, "}\n");
}

static void formatter_format_function(Formatter* formatter, FunctionDecl* func) {
    formatter_append(formatter, "fn ");
    formatter_append(formatter, func->name);
    formatter_append(formatter, "() -> String");

    if (func->body) {
        formatter_format_block(formatter, func->body);
    } else {
        formatter_append(formatter, " {\n    // TODO: function body\n}\n");
    }

    formatter_append(formatter, "\n");
}

static void formatter_format_api_route(Formatter* formatter, ApiRoute* api) {
    formatter_append(formatter, "api ");
    formatter_append(formatter, api->method ? api->method : "get");
    formatter_append(formatter, " \"");
    formatter_append(formatter, api->path ? api->path : "/");
    formatter_append(formatter, "\" () -> String {\n");

    if (api->handler && api->handler->body && api->handler->body->result_expr) {
        formatter->indent_level++;
        formatter_append_indent(formatter);
        formatter_format_expr(formatter, api->handler->body->result_expr);
        formatter->indent_level--;
    }

    formatter_append(formatter, "\n}\n\n");
}

Formatter* formatter_create(void) {
    Formatter* formatter = calloc(1, sizeof(Formatter));
    if (!formatter) return NULL;

    formatter->buffer_capacity = INITIAL_BUFFER_SIZE;
    formatter->buffer = malloc(formatter->buffer_capacity);
    if (!formatter->buffer) {
        free(formatter);
        return NULL;
    }

    formatter->buffer[0] = '\0';
    formatter->buffer_size = 0;
    formatter->indent_level = 0;

    return formatter;
}

void formatter_free(Formatter* formatter) {
    if (formatter) {
        free(formatter->buffer);
        free(formatter);
    }
}

void formatter_format_program(Formatter* formatter, Program* program) {
    for (size_t i = 0; i < program->module_count; i++) {
        Module* module = program->modules[i];

        // Format API routes first
        for (size_t j = 0; j < module->api_route_count; j++) {
            formatter_format_api_route(formatter, module->api_routes[j]);
        }

        // Format functions
        for (size_t j = 0; j < module->function_count; j++) {
            formatter_format_function(formatter, module->functions[j]);
        }
    }
}

char* formatter_get_code(Formatter* formatter) {
    return formatter->buffer;
}

