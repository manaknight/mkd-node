#include "js_emitter.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define INITIAL_BUFFER_SIZE 1024
#define BUFFER_GROWTH_FACTOR 2

static void js_emitter_grow_buffer(JSEmitter* emitter, size_t needed) {
    while (emitter->buffer_capacity < needed) {
        emitter->buffer_capacity *= BUFFER_GROWTH_FACTOR;
    }
    emitter->buffer = realloc(emitter->buffer, emitter->buffer_capacity);
}

static void js_emitter_append(JSEmitter* emitter, const char* str) {
    size_t len = strlen(str);
    size_t needed = emitter->buffer_size + len + 1;

    if (needed > emitter->buffer_capacity) {
        js_emitter_grow_buffer(emitter, needed);
    }

    strcpy(emitter->buffer + emitter->buffer_size, str);
    emitter->buffer_size += len;
}

static void js_emitter_emit_literal(JSEmitter* emitter, Literal* literal) {
    char buf[32];
    switch (literal->kind) {
        case LIT_STRING:
            js_emitter_append(emitter, "\"");
            js_emitter_append(emitter, literal->value.string_val);
            js_emitter_append(emitter, "\"");
            break;
        case LIT_INT64:
            sprintf(buf, "%lld", (long long)literal->value.int64_val);
            js_emitter_append(emitter, buf);
            break;
        case LIT_BOOL:
            js_emitter_append(emitter, literal->value.bool_val ? "true" : "false");
            break;
        case LIT_UNIT:
            js_emitter_append(emitter, "undefined");
            break;
    }
}

static void js_emitter_emit_expr(JSEmitter* emitter, void* expr_node);
static void js_emitter_emit_function(JSEmitter* emitter, FunctionDecl* func);

static void js_emitter_emit_call(JSEmitter* emitter, CallExpr* call) {
    js_emitter_emit_expr(emitter, call->function);
    js_emitter_append(emitter, "(");

    for (size_t i = 0; i < call->argument_count; i++) {
        if (i > 0) js_emitter_append(emitter, ", ");
        js_emitter_emit_expr(emitter, call->arguments[i]);
    }

    js_emitter_append(emitter, ")");
}

static void js_emitter_emit_expr(JSEmitter* emitter, void* expr_node) {
    AstNode* node = (AstNode*)expr_node;

    switch (node->type) {
        case NODE_LITERAL:
            js_emitter_emit_literal(emitter, (Literal*)expr_node);
            break;
        case NODE_IDENTIFIER_EXPR:
            js_emitter_append(emitter, ((IdentifierExpr*)expr_node)->name);
            break;
        case NODE_CALL_EXPR:
            js_emitter_emit_call(emitter, (CallExpr*)expr_node);
            break;
        // TODO: Add more expression types
        default:
            js_emitter_append(emitter, "/* TODO: unimplemented expr */");
            break;
    }
}

static void js_emitter_emit_block(JSEmitter* emitter, Block* block) {
    js_emitter_append(emitter, "{\n");

    for (size_t i = 0; i < block->statement_count; i++) {
        // TODO: Handle different statement types
        js_emitter_append(emitter, "    // TODO: statement\n");
    }

    if (block->result_expr) {
        js_emitter_append(emitter, "    return ");
        js_emitter_emit_expr(emitter, block->result_expr);
        js_emitter_append(emitter, ";\n");
    }

    js_emitter_append(emitter, "}\n");
}

static void js_emitter_emit_api_route(JSEmitter* emitter, ApiRoute* route) {
    // For now, just emit the handler function
    // TODO: Generate proper HTTP server setup
    js_emitter_append(emitter, "// API route: ");
    js_emitter_append(emitter, route->method);
    js_emitter_append(emitter, " ");
    js_emitter_append(emitter, route->path);
    js_emitter_append(emitter, "\n");

    // Emit the handler function
    js_emitter_emit_function(emitter, route->handler);
}

void js_emitter_emit_function(JSEmitter* emitter, FunctionDecl* func) {
    js_emitter_append(emitter, "function ");
    js_emitter_append(emitter, func->name);
    js_emitter_append(emitter, "(");

    // For now, ignore parameters and effects
    js_emitter_append(emitter, ") ");

    if (func->body) {
        js_emitter_emit_block(emitter, func->body);
    } else {
        js_emitter_append(emitter, "{ /* TODO: function body */ }\n");
    }

    js_emitter_append(emitter, "\n");
}

JSEmitter* js_emitter_create(void) {
    JSEmitter* emitter = calloc(1, sizeof(JSEmitter));
    if (!emitter) return NULL;

    emitter->buffer_capacity = INITIAL_BUFFER_SIZE;
    emitter->buffer = malloc(emitter->buffer_capacity);
    if (!emitter->buffer) {
        free(emitter);
        return NULL;
    }

    emitter->buffer[0] = '\0';
    emitter->buffer_size = 0;

    return emitter;
}

void js_emitter_free(JSEmitter* emitter) {
    if (emitter) {
        free(emitter->buffer);
        free(emitter);
    }
}

void js_emitter_emit_program(JSEmitter* emitter, Program* program) {
    js_emitter_append(emitter, "\"use strict\";\n\n");
    js_emitter_append(emitter, "// Manaknight compiled code\n\n");

    // Find and emit all functions and API routes, then look for main
    FunctionDecl* main_func = NULL;
    int has_api_routes = 0;

    for (size_t i = 0; i < program->module_count; i++) {
        Module* module = program->modules[i];

        // Emit API routes
        for (size_t j = 0; j < module->api_route_count; j++) {
            ApiRoute* route = module->api_routes[j];
            js_emitter_emit_api_route(emitter, route);
            has_api_routes = 1;
        }

        // Emit functions
        for (size_t j = 0; j < module->function_count; j++) {
            FunctionDecl* func = module->functions[j];
            js_emitter_emit_function(emitter, func);

            if (strcmp(func->name, "main") == 0) {
                main_func = func;
            }
        }
    }

    // Always try to call main function for mqjs runtime
    if (main_func) {
        js_emitter_append(emitter, "\n// Call main function\n");
        js_emitter_append(emitter, "console.log(main());\n");
    } else {
        js_emitter_append(emitter, "\n// No main function found\n");
        js_emitter_append(emitter, "console.log(\"No main function defined\");\n");
    }
}

char* js_emitter_get_code(JSEmitter* emitter) {
    return emitter->buffer;
}

