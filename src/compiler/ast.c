#include "ast.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// Helper function to duplicate strings
static char* strdup_safe(const char* str) {
    if (!str) return NULL;
    return strdup(str);
}

// Helper function to duplicate string arrays
static char** strdup_array(char** arr, size_t count) {
    if (!arr || count == 0) return NULL;
    char** result = malloc(sizeof(char*) * count);
    for (size_t i = 0; i < count; i++) {
        result[i] = strdup_safe(arr[i]);
    }
    return result;
}

// Helper function to duplicate type arrays
static Type** dup_type_array(Type** arr, size_t count) {
    if (!arr || count == 0) return NULL;
    Type** result = malloc(sizeof(Type*) * count);
    for (size_t i = 0; i < count; i++) {
        // Note: This is a shallow copy - types should be immutable
        result[i] = arr[i];
    }
    return result;
}

// Helper function to duplicate param arrays
static Param* dup_param_array(Param* arr, size_t count) {
    if (!arr || count == 0) return NULL;
    Param* result = malloc(sizeof(Param) * count);
    for (size_t i = 0; i < count; i++) {
        result[i].name = strdup_safe(arr[i].name);
        result[i].type = arr[i].type; // shallow copy
    }
    return result;
}

// Program constructor
Program* create_program(Module** modules, size_t module_count, ApiRoute** apis, size_t api_count) {
    Program* program = malloc(sizeof(Program));
    program->kind = NODE_PROGRAM;
    program->modules = modules ? malloc(sizeof(Module*) * module_count) : NULL;
    if (modules) {
        memcpy(program->modules, modules, sizeof(Module*) * module_count);
    }
    program->module_count = module_count;
    program->apis = apis ? malloc(sizeof(ApiRoute*) * api_count) : NULL;
    if (apis) {
        memcpy(program->apis, apis, sizeof(ApiRoute*) * api_count);
    }
    program->api_count = api_count;
    return program;
}

// Module constructor
Module* create_module(char* name, Decl** decls, size_t decl_count) {
    Module* module = malloc(sizeof(Module));
    module->kind = NODE_MODULE;
    module->name = strdup_safe(name);
    module->decls = decls ? malloc(sizeof(Decl*) * decl_count) : NULL;
    if (decls) {
        memcpy(module->decls, decls, sizeof(Decl*) * decl_count);
    }
    module->decl_count = decl_count;
    return module;
}

// API route constructor
ApiRoute* create_api_route(char* method, char* path, Stmt* body) {
    ApiRoute* api = malloc(sizeof(ApiRoute));
    api->kind = NODE_API_ROUTE;
    api->method = strdup_safe(method);
    api->path = strdup_safe(path);
    api->body = body;
    return api;
}

// Declaration constructors
Decl* create_function_decl(char* name, Param* params, size_t param_count, Type* return_type,
                          char** effects, size_t effect_count, Stmt* body) {
    Decl* decl = malloc(sizeof(Decl));
    decl->kind = NODE_FUNCTION_DECL;
    decl->data.function.name = strdup_safe(name);
    decl->data.function.params = dup_param_array(params, param_count);
    decl->data.function.param_count = param_count;
    decl->data.function.return_type = return_type;
    decl->data.function.effects = strdup_array(effects, effect_count);
    decl->data.function.effect_count = effect_count;
    decl->data.function.body = body;
    return decl;
}

Decl* create_type_decl(char* name, bool is_union, void* body_data) {
    Decl* decl = malloc(sizeof(Decl));
    decl->kind = NODE_TYPE_DECL;
    decl->data.type.name = strdup_safe(name);
    decl->data.type.is_union = is_union;

    if (is_union) {
        // For union types, body_data should be a pointer to union body
        // This is a simplified version - in practice you'd need more structure
        memcpy(&decl->data.type.body, body_data, sizeof(decl->data.type.body));
    } else {
        // For record types
        memcpy(&decl->data.type.body, body_data, sizeof(decl->data.type.body));
    }
    return decl;
}

Decl* create_effect_decl(char* name) {
    Decl* decl = malloc(sizeof(Decl));
    decl->kind = NODE_EFFECT_DECL;
    decl->data.effect.name = strdup_safe(name);
    return decl;
}

Decl* create_import_decl(char* module_path, char* alias) {
    Decl* decl = malloc(sizeof(Decl));
    decl->kind = NODE_IMPORT_DECL;
    decl->data.import.module_path = strdup_safe(module_path);
    decl->data.import.alias = strdup_safe(alias);
    return decl;
}

// Statement constructors
Stmt* create_let_stmt(char* name, Expr* expr) {
    Stmt* stmt = malloc(sizeof(Stmt));
    stmt->kind = NODE_LET_STMT;
    stmt->data.let.name = strdup_safe(name);
    stmt->data.let.expr = expr;
    return stmt;
}

Stmt* create_expr_stmt(Expr* expr) {
    Stmt* stmt = malloc(sizeof(Stmt));
    stmt->kind = NODE_EXPR_STMT;
    stmt->data.expr_stmt.expr = expr;
    return stmt;
}

Stmt* create_if_stmt(Expr* condition, Stmt* then_branch, Stmt* else_branch) {
    Stmt* stmt = malloc(sizeof(Stmt));
    stmt->kind = NODE_IF_STMT;
    stmt->data.if_stmt.condition = condition;
    stmt->data.if_stmt.then_branch = then_branch;
    stmt->data.if_stmt.else_branch = else_branch;
    return stmt;
}

Stmt* create_match_stmt(Expr* scrutinee, MatchCase* cases, size_t case_count) {
    Stmt* stmt = malloc(sizeof(Stmt));
    stmt->kind = NODE_MATCH_STMT;
    stmt->data.match.scrutinee = scrutinee;
    stmt->data.match.cases = malloc(sizeof(MatchCase) * case_count);
    memcpy(stmt->data.match.cases, cases, sizeof(MatchCase) * case_count);
    stmt->data.match.case_count = case_count;
    return stmt;
}

Stmt* create_block(Stmt** statements, size_t stmt_count) {
    // For blocks, we create a special statement that holds other statements
    // This is a simplified approach - in a real implementation you might want a dedicated Block node
    if (stmt_count == 1) {
        return statements[0]; // Single statement block
    }

    // For now, we'll represent blocks as a sequence of statements
    // This is a placeholder - proper block handling would need more structure
    return statements[0]; // Return first statement as placeholder
}

// Expression constructors
Expr* create_literal_expr(LiteralKind kind, LiteralValue value) {
    Expr* expr = malloc(sizeof(Expr));
    expr->kind = NODE_LITERAL;
    expr->data.literal.lit_kind = kind;
    expr->data.literal.value = value;
    return expr;
}

Expr* create_identifier_expr(char* name) {
    Expr* expr = malloc(sizeof(Expr));
    expr->kind = NODE_IDENTIFIER_EXPR;
    expr->data.identifier.name = strdup_safe(name);
    return expr;
}

Expr* create_call_expr(Expr* callee, Expr** args, size_t arg_count) {
    Expr* expr = malloc(sizeof(Expr));
    expr->kind = NODE_CALL_EXPR;
    expr->data.call.callee = callee;
    expr->data.call.args = args ? malloc(sizeof(Expr*) * arg_count) : NULL;
    if (args) {
        memcpy(expr->data.call.args, args, sizeof(Expr*) * arg_count);
    }
    expr->data.call.arg_count = arg_count;
    return expr;
}

Expr* create_lambda_expr(Param* params, size_t param_count, Expr* body) {
    Expr* expr = malloc(sizeof(Expr));
    expr->kind = NODE_LAMBDA_EXPR;
    expr->data.lambda.params = dup_param_array(params, param_count);
    expr->data.lambda.param_count = param_count;
    expr->data.lambda.body = body;
    return expr;
}

Expr* create_if_expr(Expr* condition, Expr* then_branch, Expr* else_branch) {
    Expr* expr = malloc(sizeof(Expr));
    expr->kind = NODE_IF_EXPR;
    expr->data.if_expr.condition = condition;
    expr->data.if_expr.then_branch = then_branch;
    expr->data.if_expr.else_branch = else_branch;
    return expr;
}

Expr* create_match_expr(Expr* scrutinee, MatchCase* cases, size_t case_count) {
    Expr* expr = malloc(sizeof(Expr));
    expr->kind = NODE_MATCH_EXPR;
    expr->data.match.scrutinee = scrutinee;
    expr->data.match.cases = malloc(sizeof(MatchCase) * case_count);
    memcpy(expr->data.match.cases, cases, sizeof(MatchCase) * case_count);
    expr->data.match.case_count = case_count;
    return expr;
}

Expr* create_pipe_expr(Expr* left, Expr* right) {
    Expr* expr = malloc(sizeof(Expr));
    expr->kind = NODE_PIPE_EXPR;
    expr->data.pipe.left = left;
    expr->data.pipe.right = right;
    return expr;
}

// Type constructors
Type* create_primitive_type(PrimitiveTypeKind kind) {
    Type* type = malloc(sizeof(Type));
    type->kind = NODE_PRIMITIVE_TYPE;
    type->data.primitive.primitive_kind = kind;
    return type;
}

Type* create_named_type(char* name) {
    Type* type = malloc(sizeof(Type));
    type->kind = NODE_NAMED_TYPE;
    type->data.named.name = strdup_safe(name);
    return type;
}

Type* create_generic_type(char* name, Type** args, size_t arg_count) {
    Type* type = malloc(sizeof(Type));
    type->kind = NODE_GENERIC_TYPE;
    type->data.generic.name = strdup_safe(name);
    type->data.generic.args = dup_type_array(args, arg_count);
    type->data.generic.arg_count = arg_count;
    return type;
}

Type* create_function_type(Type** params, size_t param_count, Type* return_type,
                          char** effects, size_t effect_count) {
    Type* type = malloc(sizeof(Type));
    type->kind = NODE_FUNCTION_TYPE;
    type->data.function.params = dup_type_array(params, param_count);
    type->data.function.param_count = param_count;
    type->data.function.return_type = return_type;
    type->data.function.effects = strdup_array(effects, effect_count);
    type->data.function.effect_count = effect_count;
    return type;
}

// Pattern constructors
Pattern* create_constructor_pattern(char* constructor, Param* fields, size_t field_count) {
    Pattern* pattern = malloc(sizeof(Pattern));
    pattern->kind = NODE_CONSTRUCTOR_PATTERN;
    pattern->data.constructor.constructor = strdup_safe(constructor);
    pattern->data.constructor.fields = dup_param_array(fields, field_count);
    pattern->data.constructor.field_count = field_count;
    return pattern;
}

Pattern* create_wildcard_pattern() {
    Pattern* pattern = malloc(sizeof(Pattern));
    pattern->kind = NODE_WILDCARD_PATTERN;
    return pattern;
}

// Memory cleanup functions (simplified - in practice would need proper traversal)
void free_ast(Program* program) {
    if (!program) return;

    // Free modules
    for (size_t i = 0; i < program->module_count; i++) {
        free(program->modules[i]->name);
        // TODO: Free declarations recursively
        free(program->modules[i]);
    }
    free(program->modules);

    // Free APIs
    for (size_t i = 0; i < program->api_count; i++) {
        free(program->apis[i]->method);
        free(program->apis[i]->path);
        // TODO: Free body recursively
        free(program->apis[i]);
    }
    free(program->apis);

    free(program);
}

// Debug string representation (simplified)
char* ast_to_string(Program* program) {
    if (!program) return strdup("null");

    char* result = malloc(1024); // Simplified - real implementation would need dynamic sizing
    snprintf(result, 1024, "Program { modules: %zu, apis: %zu }",
             program->module_count, program->api_count);
    return result;
}
