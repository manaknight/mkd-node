#include "ast.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// AST construction functions
Program* ast_create_program(void) {
    Program* program = calloc(1, sizeof(Program));
    if (program) {
        program->base.type = NODE_PROGRAM;
        program->base.line = 1;
        program->base.column = 1;
    }
    return program;
}

void ast_free_program(Program* program) {
    if (!program) return;

    for (size_t i = 0; i < program->module_count; i++) {
        // TODO: Free individual modules
        free(program->modules[i]);
    }
    free(program->modules);
    free(program);
}

// Utility functions
void ast_free_node(AstNode* node) {
    if (!node) return;

    // TODO: Implement proper node freeing based on type
    free(node);
}

char* ast_node_to_string(AstNode* node) {
    if (!node) return strdup("(null)");

    // TODO: Implement proper string conversion
    char buffer[256];
    snprintf(buffer, sizeof(buffer), "Node(type=%d, line=%u, col=%u)",
             node->type, node->line, node->column);
    return strdup(buffer);
}