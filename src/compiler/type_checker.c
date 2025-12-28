#include "type_checker.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// Type checker creation and destruction
TypeChecker* create_type_checker(SymbolTable* symbol_table) {
    TypeChecker* checker = malloc(sizeof(TypeChecker));
    checker->symbol_table = symbol_table;
    checker->has_errors = false;
    return checker;
}

void free_type_checker(TypeChecker* checker) {
    free(checker);
}

// Main type checking entry point
bool type_check_program(TypeChecker* checker, Program* program) {
    checker->has_errors = false;

    // Load prelude symbols
    load_prelude(checker->symbol_table);

    // Type check all modules
    for (size_t i = 0; i < program->module_count; i++) {
        if (!type_check_module(checker, program->modules[i])) {
            checker->has_errors = true;
        }
    }

    // Type check all APIs
    for (size_t i = 0; i < program->api_count; i++) {
        // APIs have their own type checking rules
        // For now, just check the body
        if (program->apis[i]->body) {
            enter_scope(checker->symbol_table, "api");
            if (!type_check_statement(checker, program->apis[i]->body)) {
                checker->has_errors = true;
            }
            exit_scope(checker->symbol_table);
        }
    }

    return !checker->has_errors;
}

bool type_check_module(TypeChecker* checker, Module* module) {
    // Enter module scope
    enter_scope(checker->symbol_table, module->name);

    // First pass: declare all symbols
    for (size_t i = 0; i < module->decl_count; i++) {
        Decl* decl = module->decls[i];

        switch (decl->kind) {
            case NODE_FUNCTION_DECL: {
                // Declare function symbol
                Symbol* func_symbol = create_symbol(
                    decl->data.function.name,
                    SYMBOL_FUNCTION,
                    create_function_type(
                        NULL, 0, // params will be filled during checking
                        decl->data.function.return_type,
                        (char**)decl->data.function.effects,
                        decl->data.function.effect_count
                    ),
                    decl
                );
                if (!declare_symbol(checker->symbol_table, func_symbol)) {
                    checker->has_errors = true;
                }
                break;
            }
            case NODE_TYPE_DECL: {
                // Declare type symbol
                Symbol* type_symbol = create_symbol(
                    decl->data.type.name,
                    SYMBOL_TYPE,
                    NULL, // Types don't have types themselves
                    decl
                );
                if (!declare_symbol(checker->symbol_table, type_symbol)) {
                    checker->has_errors = true;
                }
                break;
            }
            case NODE_EFFECT_DECL: {
                // Declare effect symbol
                Symbol* effect_symbol = create_symbol(
                    decl->data.effect.name,
                    SYMBOL_EFFECT,
                    NULL,
                    decl
                );
                if (!declare_symbol(checker->symbol_table, effect_symbol)) {
                    checker->has_errors = true;
                }
                break;
            }
            default:
                break;
        }
    }

    // Second pass: type check declarations
    for (size_t i = 0; i < module->decl_count; i++) {
        if (!type_check_declaration(checker, module->decls[i])) {
            checker->has_errors = true;
        }
    }

    exit_scope(checker->symbol_table);
    return !checker->has_errors;
}

bool type_check_declaration(TypeChecker* checker, Decl* decl) {
    switch (decl->kind) {
        case NODE_FUNCTION_DECL:
            return type_check_function_declaration(checker, decl);
        case NODE_TYPE_DECL:
            return type_check_type_declaration(checker, decl);
        case NODE_EFFECT_DECL:
            return type_check_effect_declaration(checker, decl);
        case NODE_IMPORT_DECL:
            return type_check_import_declaration(checker, decl);
        default:
            return false;
    }
}

// Type checking for expressions
Type* type_check_expression(TypeChecker* checker, Expr* expr) {
    switch (expr->kind) {
        case NODE_LITERAL:
            return type_check_literal(checker, expr);
        case NODE_IDENTIFIER_EXPR:
            return type_check_identifier(checker, expr);
        case NODE_CALL_EXPR:
            return type_check_call(checker, expr);
        case NODE_IF_EXPR:
            return type_check_if(checker, expr);
        case NODE_MATCH_EXPR:
            return type_check_match(checker, expr);
        case NODE_PIPE_EXPR:
            return type_check_pipe(checker, expr);
        default:
            report_type_error_at(checker, E2001_UNKNOWN_IDENTIFIER,
                               "Unknown expression type", 0, 0);
            return NULL;
    }
}

Type* type_check_literal(TypeChecker* checker, Expr* expr) {
    switch (expr->data.literal.lit_kind) {
        case LIT_INT:
            return create_primitive_type(PRIM_INT);
        case LIT_BOOL:
            return create_primitive_type(PRIM_BOOL);
        case LIT_STRING:
            return create_primitive_type(PRIM_STRING);
        default:
            return NULL;
    }
}

Type* type_check_identifier(TypeChecker* checker, Expr* expr) {
    Symbol* symbol = resolve_symbol(checker->symbol_table, expr->data.identifier.name);
    if (!symbol) {
        report_type_error_at(checker, E2001_UNKNOWN_IDENTIFIER,
                           "Unknown identifier", 0, 0);
        return NULL;
    }
    return resolve_type(symbol->type, checker->symbol_table);
}

Type* type_check_call(TypeChecker* checker, Expr* expr) {
    // Type check the callee
    Type* callee_type = type_check_expression(checker, expr->data.call.callee);
    if (!callee_type || callee_type->kind != NODE_FUNCTION_TYPE) {
        report_type_error_at(checker, E2003_INVALID_FUNCTION_CALL,
                           "Cannot call non-function", 0, 0);
        return NULL;
    }

    // Check argument count
    if (expr->data.call.arg_count != callee_type->data.function.param_count) {
        report_type_error_at(checker, E2003_INVALID_FUNCTION_CALL,
                           "Wrong number of arguments", 0, 0);
        return NULL;
    }

    // Check argument types
    for (size_t i = 0; i < expr->data.call.arg_count; i++) {
        Type* arg_type = type_check_expression(checker, expr->data.call.args[i]);
        if (!arg_type || !types_equal(arg_type, callee_type->data.function.params[i])) {
            report_type_error_at(checker, E2002_TYPE_MISMATCH,
                               "Argument type mismatch", 0, 0);
            return NULL;
        }
    }

    return resolve_type(callee_type->data.function.return_type, checker->symbol_table);
}

Type* type_check_if(TypeChecker* checker, Expr* expr) {
    // Check condition is Bool
    Type* cond_type = type_check_expression(checker, expr->data.if_expr.condition);
    if (!cond_type || cond_type->kind != NODE_PRIMITIVE_TYPE ||
        cond_type->data.primitive.primitive_kind != PRIM_BOOL) {
        report_type_error_at(checker, E2007_INVALID_CONDITION_TYPE,
                           "If condition must be Bool", 0, 0);
        return NULL;
    }

    // Check both branches have the same type
    Type* then_type = type_check_expression(checker, expr->data.if_expr.then_branch);
    Type* else_type = type_check_expression(checker, expr->data.if_expr.else_branch);

    if (!then_type || !else_type || !types_equal(then_type, else_type)) {
        report_type_error_at(checker, E2002_TYPE_MISMATCH,
                           "If branches must have the same type", 0, 0);
        return NULL;
    }

    return then_type;
}

Type* type_check_match(TypeChecker* checker, Expr* expr) {
    // Check scrutinee is an ADT type
    Type* scrutinee_type = type_check_expression(checker, expr->data.match.scrutinee);
    if (!scrutinee_type) return NULL;

    // For now, assume ADT checking - this would need more implementation
    // to check exhaustiveness and unify branch types

    // Check that all branches return the same type
    Type* result_type = NULL;
    for (size_t i = 0; i < expr->data.match.case_count; i++) {
        Type* branch_type = type_check_expression(checker, expr->data.match.cases[i].body);
        if (!branch_type) return NULL;

        if (!result_type) {
            result_type = branch_type;
        } else if (!types_equal(result_type, branch_type)) {
            report_type_error_at(checker, E4004_INCONSISTENT_MATCH_RESULT_TYPES,
                               "Match branches must return the same type", 0, 0);
            return NULL;
        }
    }

    return result_type;
}

Type* type_check_pipe(TypeChecker* checker, Expr* expr) {
    // Pipe is syntactic sugar: a |> f becomes f(a)
    // For type checking, we can just check both sides
    Type* left_type = type_check_expression(checker, expr->data.pipe.left);
    Type* right_type = type_check_expression(checker, expr->data.pipe.right);

    // The right side should be a function that accepts the left type
    if (!left_type || !right_type || right_type->kind != NODE_FUNCTION_TYPE) {
        report_type_error_at(checker, E2003_INVALID_FUNCTION_CALL,
                           "Right side of pipe must be a function", 0, 0);
        return NULL;
    }

    // Check that function accepts the left type as first argument
    if (right_type->data.function.param_count == 0 ||
        !types_equal(left_type, right_type->data.function.params[0])) {
        report_type_error_at(checker, E2002_TYPE_MISMATCH,
                           "Pipe argument type mismatch", 0, 0);
        return NULL;
    }

    return resolve_type(right_type->data.function.return_type, checker->symbol_table);
}

// Type checking for statements
bool type_check_statement(TypeChecker* checker, Stmt* stmt) {
    switch (stmt->kind) {
        case NODE_LET_STMT:
            return type_check_let_statement(checker, stmt);
        case NODE_EXPR_STMT:
            return type_check_expression(checker, stmt->data.expr_stmt.expr) != NULL;
        case NODE_IF_STMT:
            return type_check_if_statement(checker, stmt);
        case NODE_MATCH_STMT:
            return type_check_match_statement(checker, stmt);
        default:
            return false;
    }
}

bool type_check_let_statement(TypeChecker* checker, Stmt* stmt) {
    // Infer type from expression
    Type* expr_type = type_check_expression(checker, stmt->data.let.expr);
    if (!expr_type) return false;

    // Declare the variable in current scope
    Symbol* var_symbol = create_symbol(stmt->data.let.name, SYMBOL_VARIABLE, expr_type, stmt);
    if (!declare_symbol(checker->symbol_table, var_symbol)) {
        checker->has_errors = true;
        return false;
    }

    return true;
}

bool type_check_if_statement(TypeChecker* checker, Stmt* stmt) {
    // Check condition
    Type* cond_type = type_check_expression(checker, stmt->data.if_stmt.condition);
    if (!cond_type || cond_type->kind != NODE_PRIMITIVE_TYPE ||
        cond_type->data.primitive.primitive_kind != PRIM_BOOL) {
        report_type_error_at(checker, E2007_INVALID_CONDITION_TYPE,
                           "If condition must be Bool", 0, 0);
        return false;
    }

    // Check then branch
    if (!type_check_statement(checker, stmt->data.if_stmt.then_branch)) {
        return false;
    }

    // Check else branch if present
    if (stmt->data.if_stmt.else_branch) {
        if (!type_check_statement(checker, stmt->data.if_stmt.else_branch)) {
            return false;
        }
    }

    return true;
}

bool type_check_match_statement(TypeChecker* checker, Stmt* stmt) {
    // Similar to expression match but for statements
    Type* scrutinee_type = type_check_expression(checker, stmt->data.match.scrutinee);
    if (!scrutinee_type) return false;

    // Check each case body
    for (size_t i = 0; i < stmt->data.match.case_count; i++) {
        if (!type_check_statement(checker, (Stmt*)stmt->data.match.cases[i].body)) {
            return false;
        }
    }

    return check_match_exhaustiveness(checker, (Expr*)&stmt->data.match);
}

// Declaration type checking
bool type_check_function_declaration(TypeChecker* checker, Decl* decl) {
    // Enter function scope
    enter_scope(checker->symbol_table, decl->data.function.name);

    // Declare parameters
    for (size_t i = 0; i < decl->data.function.param_count; i++) {
        Symbol* param_symbol = create_symbol(
            decl->data.function.params[i].name,
            SYMBOL_VARIABLE,
            decl->data.function.params[i].type,
            decl
        );
        if (!declare_symbol(checker->symbol_table, param_symbol)) {
            exit_scope(checker->symbol_table);
            return false;
        }
    }

    // Type check function body
    bool body_ok = type_check_statement(checker, decl->data.function.body);

    // Check totality (all paths return a value)
    if (!check_control_flow_totality(checker, decl->data.function.body)) {
        report_type_error_at(checker, E2005_MISSING_RETURN_VALUE,
                           "Function must return a value on all paths", 0, 0);
        body_ok = false;
    }

    exit_scope(checker->symbol_table);
    return body_ok;
}

bool type_check_type_declaration(TypeChecker* checker, Decl* decl) {
    // Type declarations are mostly just declarations
    // More complex checking would be needed for ADT validation
    return true;
}

bool type_check_effect_declaration(TypeChecker* checker, Decl* decl) {
    // Effect declarations are just declarations
    return true;
}

bool type_check_import_declaration(TypeChecker* checker, Decl* decl) {
    // Import declarations would need module resolution
    // For now, assume they're valid
    return true;
}

// Type operations
bool types_equal(Type* a, Type* b) {
    if (!a || !b) return false;
    if (a->kind != b->kind) return false;

    switch (a->kind) {
        case NODE_PRIMITIVE_TYPE:
            return a->data.primitive.primitive_kind == b->data.primitive.primitive_kind;
        case NODE_NAMED_TYPE:
            return strcmp(a->data.named.name, b->data.named.name) == 0;
        case NODE_GENERIC_TYPE:
            return strcmp(a->data.generic.name, b->data.generic.name) == 0 &&
                   a->data.generic.arg_count == b->data.generic.arg_count &&
                   type_arrays_equal(a->data.generic.args, b->data.generic.args, a->data.generic.arg_count);
        case NODE_FUNCTION_TYPE:
            return a->data.function.param_count == b->data.function.param_count &&
                   type_arrays_equal(a->data.function.params, b->data.function.params, a->data.function.param_count) &&
                   types_equal(a->data.function.return_type, b->data.function.return_type);
        default:
            return false;
    }
}

bool type_arrays_equal(Type** a, Type** b, size_t count) {
    for (size_t i = 0; i < count; i++) {
        if (!types_equal(a[i], b[i])) return false;
    }
    return true;
}

Type* resolve_type(Type* type, SymbolTable* symbols) {
    if (!type) return NULL;

    // For named types, look up in symbol table
    if (type->kind == NODE_NAMED_TYPE) {
        Symbol* symbol = resolve_symbol(symbols, type->data.named.name);
        if (symbol && symbol->kind == SYMBOL_TYPE) {
            return symbol->type;
        }
    }

    return type;
}

bool is_numeric_type(Type* type) {
    return type && type->kind == NODE_PRIMITIVE_TYPE &&
           type->data.primitive.primitive_kind == PRIM_INT;
}

bool is_comparable_type(Type* type) {
    return is_numeric_type(type) ||
           (type && type->kind == NODE_PRIMITIVE_TYPE &&
            type->data.primitive.primitive_kind == PRIM_STRING);
}

// Utility functions
void report_type_error_at(TypeChecker* checker, TypeError code, const char* message,
                         uint32_t line, uint32_t column) {
    checker->has_errors = true;
    report_type_error(code, message, NULL, line, column);
}

bool check_function_call_args(TypeChecker* checker, Expr* call_expr, Symbol* func_symbol) {
    // Implementation would check function signature against call arguments
    return true; // Placeholder
}

bool check_match_exhaustiveness(TypeChecker* checker, Expr* match_expr) {
    // Implementation would check that all ADT constructors are covered
    return true; // Placeholder
}

bool check_control_flow_totality(TypeChecker* checker, Stmt* stmt) {
    // Implementation would check that all control flow paths return a value
    return true; // Placeholder
}
