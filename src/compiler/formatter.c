#include "formatter.h"
#include <stdlib.h>
#include <string.h>

// Formatter creation and destruction
Formatter* create_formatter(FILE* output) {
    Formatter* formatter = malloc(sizeof(Formatter));
    formatter->output = output;
    formatter->indent_level = 0;
    formatter->needs_indent = true;
    return formatter;
}

void free_formatter(Formatter* formatter) {
    free(formatter);
}

// Helper functions
void indent(Formatter* formatter) {
    formatter->indent_level += 2;
}

void dedent(Formatter* formatter) {
    formatter->indent_level -= 2;
    if (formatter->indent_level < 0) formatter->indent_level = 0;
}

void newline(Formatter* formatter) {
    fprintf(formatter->output, "\n");
    formatter->needs_indent = true;
}

void write_string(Formatter* formatter, const char* str) {
    if (formatter->needs_indent) {
        write_indent(formatter);
        formatter->needs_indent = false;
    }
    fprintf(formatter->output, "%s", str);
}

void write_indent(Formatter* formatter) {
    for (int i = 0; i < formatter->indent_level; i++) {
        fprintf(formatter->output, " ");
    }
}

// Main formatting functions
void format_program(Formatter* formatter, Program* program) {
    // Format modules
    for (size_t i = 0; i < program->module_count; i++) {
        format_module(formatter, program->modules[i]);
        if (i < program->module_count - 1 || program->api_count > 0) {
            newline(formatter);
        }
    }

    // Format APIs
    for (size_t i = 0; i < program->api_count; i++) {
        format_api(formatter, program->apis[i]);
        if (i < program->api_count - 1) {
            newline(formatter);
        }
    }
}

void format_module(Formatter* formatter, Module* module) {
    write_string(formatter, "module ");
    write_string(formatter, module->name);
    write_string(formatter, " {");
    newline(formatter);
    indent(formatter);

    // Format declarations
    for (size_t i = 0; i < module->decl_count; i++) {
        format_declaration(formatter, module->decls[i]);
        if (i < module->decl_count - 1) {
            newline(formatter);
        }
    }

    dedent(formatter);
    newline(formatter);
    write_string(formatter, "}");
}

void format_api(Formatter* formatter, ApiRoute* api) {
    write_string(formatter, "api ");
    write_string(formatter, api->method);
    write_string(formatter, " ");
    write_string(formatter, api->path);
    write_string(formatter, " {");
    newline(formatter);
    indent(formatter);

    // Format body (simplified)
    if (api->body) {
        format_statement(formatter, api->body);
    }

    dedent(formatter);
    newline(formatter);
    write_string(formatter, "}");
}

void format_declaration(Formatter* formatter, Decl* decl) {
    switch (decl->kind) {
        case NODE_FUNCTION_DECL:
            format_function_declaration(formatter, &decl->data.function);
            break;
        case NODE_TYPE_DECL:
            format_type_declaration(formatter, &decl->data.type);
            break;
        case NODE_EFFECT_DECL:
            format_effect_declaration(formatter, &decl->data.effect);
            break;
        case NODE_IMPORT_DECL:
            format_import_declaration(formatter, &decl->data.import);
            break;
        default:
            // Unknown declaration type
            break;
    }
}

void format_function_declaration(Formatter* formatter, void* func_data) {
    // Cast to function data structure
    struct {
        char* name;
        Param* params;
        size_t param_count;
        Type* return_type;
        char** effects;
        size_t effect_count;
        Stmt* body;
    }* func = func_data;

    write_string(formatter, "function ");
    write_string(formatter, func->name);
    write_string(formatter, "(");

    // Format parameters
    for (size_t i = 0; i < func->param_count; i++) {
        write_string(formatter, func->params[i].name);
        write_string(formatter, ": ");
        format_type(formatter, func->params[i].type);
        if (i < func->param_count - 1) {
            write_string(formatter, ", ");
        }
    }

    write_string(formatter, ")");

    // Return type
    if (func->return_type && func->return_type->kind != NODE_PRIMITIVE_TYPE) {
        write_string(formatter, ": ");
        format_type(formatter, func->return_type);
    }

    // Effects
    if (func->effect_count > 0) {
        write_string(formatter, " uses { ");
        for (size_t i = 0; i < func->effect_count; i++) {
            write_string(formatter, func->effects[i]);
            if (i < func->effect_count - 1) {
                write_string(formatter, ", ");
            }
        }
        write_string(formatter, " }");
    }

    write_string(formatter, " {");
    newline(formatter);
    indent(formatter);

    // Format body
    if (func->body) {
        format_statement(formatter, func->body);
    }

    dedent(formatter);
    newline(formatter);
    write_string(formatter, "}");
}

void format_type_declaration(Formatter* formatter, void* type_data) {
    // Simplified type declaration formatting
    struct {
        char* name;
        bool is_union;
    }* type_decl = type_data;

    write_string(formatter, "type ");
    write_string(formatter, type_decl->name);
    write_string(formatter, " {");
    // TODO: Format type body
    write_string(formatter, " }");
}

void format_effect_declaration(Formatter* formatter, void* effect_data) {
    struct {
        char* name;
    }* effect = effect_data;

    write_string(formatter, "effect ");
    write_string(formatter, effect->name);
}

void format_import_declaration(Formatter* formatter, void* import_data) {
    struct {
        char* module_path;
        char* alias;
    }* import_decl = import_data;

    write_string(formatter, "import ");
    write_string(formatter, import_decl->module_path);
    if (import_decl->alias) {
        write_string(formatter, " as ");
        write_string(formatter, import_decl->alias);
    }
}

void format_statement(Formatter* formatter, Stmt* stmt) {
    switch (stmt->kind) {
        case NODE_LET_STMT:
            write_string(formatter, "let ");
            write_string(formatter, stmt->data.let.name);
            write_string(formatter, " = ");
            format_expression(formatter, stmt->data.let.expr);
            break;
        case NODE_EXPR_STMT:
            format_expression(formatter, stmt->data.expr_stmt.expr);
            break;
        case NODE_IF_STMT:
            write_string(formatter, "if ");
            format_expression(formatter, stmt->data.if_stmt.condition);
            write_string(formatter, " {");
            newline(formatter);
            indent(formatter);
            format_statement(formatter, stmt->data.if_stmt.then_branch);
            dedent(formatter);
            newline(formatter);
            write_string(formatter, "}");
            if (stmt->data.if_stmt.else_branch) {
                write_string(formatter, " else {");
                newline(formatter);
                indent(formatter);
                format_statement(formatter, stmt->data.if_stmt.else_branch);
                dedent(formatter);
                newline(formatter);
                write_string(formatter, "}");
            }
            break;
        case NODE_MATCH_STMT:
            write_string(formatter, "match ");
            format_expression(formatter, stmt->data.match.scrutinee);
            write_string(formatter, " {");
            newline(formatter);
            indent(formatter);
            for (size_t i = 0; i < stmt->data.match.case_count; i++) {
                format_pattern(formatter, stmt->data.match.cases[i].pattern);
                write_string(formatter, " -> ");
                format_expression(formatter, stmt->data.match.cases[i].body);
                if (i < stmt->data.match.case_count - 1) {
                    newline(formatter);
                }
            }
            dedent(formatter);
            newline(formatter);
            write_string(formatter, "}");
            break;
        default:
            break;
    }
}

void format_expression(Formatter* formatter, Expr* expr) {
    switch (expr->kind) {
        case NODE_LITERAL:
            switch (expr->data.literal.lit_kind) {
                case LIT_INT:
                    fprintf(formatter->output, "%lld", expr->data.literal.value.int_val);
                    break;
                case LIT_BOOL:
                    write_string(formatter, expr->data.literal.value.bool_val ? "true" : "false");
                    break;
                case LIT_STRING:
                    fprintf(formatter->output, "\"%s\"", expr->data.literal.value.string_val);
                    break;
            }
            break;
        case NODE_IDENTIFIER_EXPR:
            write_string(formatter, expr->data.identifier.name);
            break;
        case NODE_CALL_EXPR:
            write_string(formatter, expr->data.call.callee->data.identifier.name);
            write_string(formatter, "(");
            for (size_t i = 0; i < expr->data.call.arg_count; i++) {
                format_expression(formatter, expr->data.call.args[i]);
                if (i < expr->data.call.arg_count - 1) {
                    write_string(formatter, ", ");
                }
            }
            write_string(formatter, ")");
            break;
        case NODE_IF_EXPR:
            write_string(formatter, "if ");
            format_expression(formatter, expr->data.if_expr.condition);
            write_string(formatter, " { ");
            format_expression(formatter, expr->data.if_expr.then_branch);
            write_string(formatter, " } else { ");
            format_expression(formatter, expr->data.if_expr.else_branch);
            write_string(formatter, " }");
            break;
        case NODE_MATCH_EXPR:
            write_string(formatter, "match ");
            format_expression(formatter, expr->data.match.scrutinee);
            write_string(formatter, " {");
            for (size_t i = 0; i < expr->data.match.case_count; i++) {
                newline(formatter);
                indent(formatter);
                format_pattern(formatter, expr->data.match.cases[i].pattern);
                write_string(formatter, " -> ");
                format_expression(formatter, expr->data.match.cases[i].body);
                dedent(formatter);
            }
            newline(formatter);
            write_string(formatter, "}");
            break;
        case NODE_PIPE_EXPR:
            format_expression(formatter, expr->data.pipe.left);
            write_string(formatter, " |> ");
            format_expression(formatter, expr->data.pipe.right);
            break;
        default:
            break;
    }
}

void format_type(Formatter* formatter, Type* type) {
    switch (type->kind) {
        case NODE_PRIMITIVE_TYPE:
            switch (type->data.primitive.primitive_kind) {
                case PRIM_INT: write_string(formatter, "Int"); break;
                case PRIM_BOOL: write_string(formatter, "Bool"); break;
                case PRIM_STRING: write_string(formatter, "String"); break;
            }
            break;
        case NODE_NAMED_TYPE:
            write_string(formatter, type->data.named.name);
            break;
        case NODE_GENERIC_TYPE:
            write_string(formatter, type->data.generic.name);
            write_string(formatter, "<");
            for (size_t i = 0; i < type->data.generic.arg_count; i++) {
                format_type(formatter, type->data.generic.args[i]);
                if (i < type->data.generic.arg_count - 1) {
                    write_string(formatter, ", ");
                }
            }
            write_string(formatter, ">");
            break;
        case NODE_FUNCTION_TYPE:
            write_string(formatter, "(");
            for (size_t i = 0; i < type->data.function.param_count; i++) {
                format_type(formatter, type->data.function.params[i]);
                if (i < type->data.function.param_count - 1) {
                    write_string(formatter, ", ");
                }
            }
            write_string(formatter, ") -> ");
            format_type(formatter, type->data.function.return_type);
            if (type->data.function.effect_count > 0) {
                write_string(formatter, " uses { ");
                for (size_t i = 0; i < type->data.function.effect_count; i++) {
                    write_string(formatter, type->data.function.effects[i]);
                    if (i < type->data.function.effect_count - 1) {
                        write_string(formatter, ", ");
                    }
                }
                write_string(formatter, " }");
            }
            break;
        default:
            break;
    }
}

void format_pattern(Formatter* formatter, Pattern* pattern) {
    switch (pattern->kind) {
        case NODE_CONSTRUCTOR_PATTERN:
            write_string(formatter, pattern->data.constructor.constructor);
            if (pattern->data.constructor.field_count > 0) {
                write_string(formatter, "(");
                for (size_t i = 0; i < pattern->data.constructor.field_count; i++) {
                    write_string(formatter, pattern->data.constructor.fields[i].name);
                    if (i < pattern->data.constructor.field_count - 1) {
                        write_string(formatter, ", ");
                    }
                }
                write_string(formatter, ")");
            }
            break;
        case NODE_WILDCARD_PATTERN:
            write_string(formatter, "_");
            break;
        default:
            break;
    }
}
