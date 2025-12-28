#ifndef TYPE_CHECKER_H
#define TYPE_CHECKER_H

#include "ast.h"
#include "symbols.h"
#include "errors.h"
#include <stdbool.h>

// Type environment structure
typedef struct {
    SymbolTable* symbol_table;
    bool has_errors;
} TypeChecker;

// Function declarations
TypeChecker* create_type_checker(SymbolTable* symbol_table);
void free_type_checker(TypeChecker* checker);

// Main type checking functions
bool type_check_program(TypeChecker* checker, Program* program);
bool type_check_module(TypeChecker* checker, Module* module);
bool type_check_declaration(TypeChecker* checker, Decl* decl);
bool type_check_statement(TypeChecker* checker, Stmt* stmt);
Type* type_check_expression(TypeChecker* checker, Expr* expr);

// Type operations
bool types_equal(Type* a, Type* b);
bool type_unify(Type* a, Type* b);
Type* resolve_type(Type* type, SymbolTable* symbols);
bool is_numeric_type(Type* type);
bool is_comparable_type(Type* type);

// Expression type checking
Type* type_check_literal(TypeChecker* checker, Expr* expr);
Type* type_check_identifier(TypeChecker* checker, Expr* expr);
Type* type_check_call(TypeChecker* checker, Expr* expr);
Type* type_check_if(TypeChecker* checker, Expr* expr);
Type* type_check_match(TypeChecker* checker, Expr* expr);
Type* type_check_pipe(TypeChecker* checker, Expr* expr);

// Statement type checking
bool type_check_let_statement(TypeChecker* checker, Stmt* stmt);
bool type_check_if_statement(TypeChecker* checker, Stmt* stmt);
bool type_check_match_statement(TypeChecker* checker, Stmt* stmt);

// Declaration type checking
bool type_check_function_declaration(TypeChecker* checker, Decl* decl);
bool type_check_type_declaration(TypeChecker* checker, Decl* decl);
bool type_check_effect_declaration(TypeChecker* checker, Decl* decl);
bool type_check_import_declaration(TypeChecker* checker, Decl* decl);

// Utility functions
void report_type_error_at(TypeChecker* checker, TypeError code, const char* message,
                         uint32_t line, uint32_t column);
bool check_function_call_args(TypeChecker* checker, Expr* call_expr, Symbol* func_symbol);
bool check_match_exhaustiveness(TypeChecker* checker, Expr* match_expr);
bool check_control_flow_totality(TypeChecker* checker, Stmt* stmt);

#endif // TYPE_CHECKER_H
