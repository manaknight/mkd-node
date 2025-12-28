#ifndef AST_H
#define AST_H

#include <stdint.h>
#include <stdbool.h>

// Forward declarations for all AST node types
typedef struct Program Program;
typedef struct Module Module;
typedef struct ApiRoute ApiRoute;
typedef struct Decl Decl;
typedef struct Stmt Stmt;
typedef struct Expr Expr;
typedef struct Pattern Pattern;
typedef struct Type Type;
typedef struct Param Param;
typedef struct MatchCase MatchCase;

// Enums for AST node kinds
typedef enum {
    // Top-level nodes
    NODE_PROGRAM,
    NODE_MODULE,
    NODE_API_ROUTE,

    // Declarations
    NODE_FUNCTION_DECL,
    NODE_TYPE_DECL,
    NODE_EFFECT_DECL,
    NODE_IMPORT_DECL,

    // Statements
    NODE_BLOCK,
    NODE_LET_STMT,
    NODE_EXPR_STMT,
    NODE_IF_STMT,
    NODE_MATCH_STMT,

    // Expressions
    NODE_LITERAL,
    NODE_IDENTIFIER_EXPR,
    NODE_CALL_EXPR,
    NODE_LAMBDA_EXPR,
    NODE_IF_EXPR,
    NODE_MATCH_EXPR,
    NODE_PIPE_EXPR,

    // Patterns
    NODE_CONSTRUCTOR_PATTERN,
    NODE_WILDCARD_PATTERN,

    // Types
    NODE_PRIMITIVE_TYPE,
    NODE_NAMED_TYPE,
    NODE_GENERIC_TYPE,
    NODE_FUNCTION_TYPE,
} NodeKind;

// Declaration types
typedef enum {
    DECL_FUNCTION,
    DECL_TYPE,
    DECL_EFFECT,
    DECL_IMPORT,
} DeclKind;

// Statement types
typedef enum {
    STMT_LET,
    STMT_EXPR,
    STMT_IF,
    STMT_MATCH,
} StmtKind;

// Expression types
typedef enum {
    EXPR_LITERAL,
    EXPR_IDENTIFIER,
    EXPR_CALL,
    EXPR_LAMBDA,
    EXPR_IF,
    EXPR_MATCH,
    EXPR_PIPE,
} ExprKind;

// Pattern types
typedef enum {
    PATTERN_CONSTRUCTOR,
    PATTERN_WILDCARD,
} PatternKind;

// Type types
typedef enum {
    TYPE_PRIMITIVE,
    TYPE_NAMED,
    TYPE_GENERIC,
    TYPE_FUNCTION,
} TypeKind;

// Primitive types
typedef enum {
    PRIM_INT,
    PRIM_BOOL,
    PRIM_STRING,
} PrimitiveTypeKind;

// Literal types
typedef enum {
    LIT_INT,
    LIT_BOOL,
    LIT_STRING,
} LiteralKind;

// Union for literal values
typedef union {
    int64_t int_val;
    bool bool_val;
    char* string_val;
} LiteralValue;

// Core AST node structures

// Parameter
struct Param {
    char* name;
    Type* type;
};

// Type definitions
struct Type {
    NodeKind kind;
    union {
        struct {
            PrimitiveTypeKind primitive_kind;
        } primitive;
        struct {
            char* name;
        } named;
        struct {
            char* name;
            Type** args;
            size_t arg_count;
        } generic;
        struct {
            Type** params;
            size_t param_count;
            Type* return_type;
            char** effects;
            size_t effect_count;
        } function;
    } data;
};

// Pattern definitions
struct Pattern {
    NodeKind kind;
    union {
        struct {
            char* constructor;
            Param* fields;
            size_t field_count;
        } constructor;
        struct {
            // Wildcard pattern has no data
        } wildcard;
    } data;
};

// Match case
struct MatchCase {
    Pattern* pattern;
    Expr* body;
};

// Expression definitions
struct Expr {
    NodeKind kind;
    union {
        struct {
            LiteralKind lit_kind;
            LiteralValue value;
        } literal;
        struct {
            char* name;
        } identifier;
        struct {
            Expr* callee;
            Expr** args;
            size_t arg_count;
        } call;
        struct {
            Param* params;
            size_t param_count;
            Expr* body;
        } lambda;
        struct {
            Expr* condition;
            Expr* then_branch;
            Expr* else_branch;
        } if_expr;
        struct {
            Expr* scrutinee;
            MatchCase* cases;
            size_t case_count;
        } match;
        struct {
            Expr* left;
            Expr* right;
        } pipe;
    } data;
};

// Statement definitions
struct Stmt {
    NodeKind kind;
    union {
        struct {
            char* name;
            Expr* expr;
        } let;
        struct {
            Expr* expr;
        } expr_stmt;
        struct {
            Expr* condition;
            Stmt* then_branch;
            Stmt* else_branch;
        } if_stmt;
        struct {
            Expr* scrutinee;
            MatchCase* cases;
            size_t case_count;
        } match;
    } data;
};

// Declaration definitions
struct Decl {
    NodeKind kind;
    union {
        struct {
            char* name;
            Param* params;
            size_t param_count;
            Type* return_type;
            char** effects;
            size_t effect_count;
            Stmt* body;
        } function;
        struct {
            char* name;
            // Type declaration body (record or union)
            union {
                struct {
                    Param* fields;
                    size_t field_count;
                } record;
                struct {
                    struct {
                        char* name;
                        Param* fields;
                        size_t field_count;
                    }* variants;
                    size_t variant_count;
                } adt_union;
            } body;
            bool is_union; // true for union, false for record
        } type;
        struct {
            char* name;
        } effect;
        struct {
            char* module_path;
            char* alias; // optional alias
        } import;
    } data;
};

// Top-level nodes
struct Module {
    NodeKind kind;
    char* name;
    Decl** decls;
    size_t decl_count;
};

struct ApiRoute {
    NodeKind kind;
    char* method; // "GET", "POST", etc.
    char* path;   // "/users/:id"
    Stmt* body;
};

struct Program {
    NodeKind kind;
    Module** modules;
    size_t module_count;
    ApiRoute** apis;
    size_t api_count;
};

// Constructor functions (to be implemented in ast.c)
Program* create_program(Module** modules, size_t module_count, ApiRoute** apis, size_t api_count);
Module* create_module(char* name, Decl** decls, size_t decl_count);
ApiRoute* create_api_route(char* method, char* path, Stmt* body);

// Declaration constructors
Decl* create_function_decl(char* name, Param* params, size_t param_count, Type* return_type,
                          char** effects, size_t effect_count, Stmt* body);
Decl* create_type_decl(char* name, bool is_union, void* body_data);
Decl* create_effect_decl(char* name);
Decl* create_import_decl(char* module_path, char* alias);

// Statement constructors
Stmt* create_block(Stmt** statements, size_t stmt_count);
Stmt* create_let_stmt(char* name, Expr* expr);
Stmt* create_expr_stmt(Expr* expr);
Stmt* create_if_stmt(Expr* condition, Stmt* then_branch, Stmt* else_branch);
Stmt* create_match_stmt(Expr* scrutinee, MatchCase* cases, size_t case_count);

// Expression constructors
Expr* create_literal_expr(LiteralKind kind, LiteralValue value);
Expr* create_identifier_expr(char* name);
Expr* create_call_expr(Expr* callee, Expr** args, size_t arg_count);
Expr* create_lambda_expr(Param* params, size_t param_count, Expr* body);
Expr* create_if_expr(Expr* condition, Expr* then_branch, Expr* else_branch);
Expr* create_match_expr(Expr* scrutinee, MatchCase* cases, size_t case_count);
Expr* create_pipe_expr(Expr* left, Expr* right);

// Type constructors
Type* create_primitive_type(PrimitiveTypeKind kind);
Type* create_named_type(char* name);
Type* create_generic_type(char* name, Type** args, size_t arg_count);
Type* create_function_type(Type** params, size_t param_count, Type* return_type,
                          char** effects, size_t effect_count);

// Pattern constructors
Pattern* create_constructor_pattern(char* constructor, Param* fields, size_t field_count);
Pattern* create_wildcard_pattern();

// Utility functions
void free_ast(Program* program);
char* ast_to_string(Program* program);

#endif // AST_H
