#ifndef MANAKNIGHT_AST_H
#define MANAKNIGHT_AST_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

// Forward declarations
typedef struct Program Program;
typedef struct Module Module;
typedef struct ApiRoute ApiRoute;
typedef struct FunctionDecl FunctionDecl;
typedef struct TypeDecl TypeDecl;
typedef struct EffectDecl EffectDecl;
typedef struct ImportDecl ImportDecl;
typedef struct Block Block;
typedef struct LetStmt LetStmt;
typedef struct ExprStmt ExprStmt;
typedef struct IfStmt IfStmt;
typedef struct MatchStmt MatchStmt;
typedef struct Literal Literal;
typedef struct IdentifierExpr IdentifierExpr;
typedef struct CallExpr CallExpr;
typedef struct LambdaExpr LambdaExpr;
typedef struct IfExpr IfExpr;
typedef struct MatchExpr MatchExpr;
typedef struct PipeExpr PipeExpr;
typedef struct ConstructorPattern ConstructorPattern;
typedef struct WildcardPattern WildcardPattern;
typedef struct PrimitiveType PrimitiveType;
typedef struct NamedType NamedType;
typedef struct GenericType GenericType;
typedef struct FunctionType FunctionType;

// AST Node types
typedef enum {
    NODE_PROGRAM,
    NODE_MODULE,
    NODE_API_ROUTE,
    NODE_FUNCTION_DECL,
    NODE_TYPE_DECL,
    NODE_EFFECT_DECL,
    NODE_IMPORT_DECL,
    NODE_BLOCK,
    NODE_LET_STMT,
    NODE_EXPR_STMT,
    NODE_IF_STMT,
    NODE_MATCH_STMT,
    NODE_LITERAL,
    NODE_IDENTIFIER_EXPR,
    NODE_CALL_EXPR,
    NODE_LAMBDA_EXPR,
    NODE_IF_EXPR,
    NODE_MATCH_EXPR,
    NODE_PIPE_EXPR,
    NODE_CONSTRUCTOR_PATTERN,
    NODE_WILDCARD_PATTERN,
    NODE_PRIMITIVE_TYPE,
    NODE_NAMED_TYPE,
    NODE_GENERIC_TYPE,
    NODE_FUNCTION_TYPE
} AstNodeType;

// Base AST node
typedef struct AstNode {
    AstNodeType type;
    uint32_t line;
    uint32_t column;
} AstNode;

// Program (top level)
struct Program {
    AstNode base;
    Module** modules;
    size_t module_count;
};

// Module
struct Module {
    AstNode base;
    char* name;
    char* path;
    ApiRoute** api_routes;
    size_t api_route_count;
    FunctionDecl** functions;
    size_t function_count;
    TypeDecl** types;
    size_t type_count;
    EffectDecl** effects;
    size_t effect_count;
    ImportDecl** imports;
    size_t import_count;
};

// API Route
struct ApiRoute {
    AstNode base;
    char* method;
    char* path;
    FunctionDecl* handler;
};

// Function Declaration
struct FunctionDecl {
    AstNode base;
    char* name;
    char** param_names;
    size_t param_count;
    char** effect_names;
    size_t effect_count;
    void* return_type; // Type node
    Block* body;
};

// Type Declaration
struct TypeDecl {
    AstNode base;
    char* name;
    char** type_params;
    size_t type_param_count;
    void* variants; // ADT constructors
};

// Effect Declaration
struct EffectDecl {
    AstNode base;
    char* name;
    void* operations; // Effect operations
};

// Import Declaration
struct ImportDecl {
    AstNode base;
    char* module_name;
    char* alias;
};

// Block
struct Block {
    AstNode base;
    void** statements; // Statement nodes
    size_t statement_count;
    void* result_expr; // Expression node (optional)
};

// Let Statement
struct LetStmt {
    AstNode base;
    char* name;
    void* expr; // Expression node
};

// Expression Statement
struct ExprStmt {
    AstNode base;
    void* expr; // Expression node
};

// If Statement
struct IfStmt {
    AstNode base;
    void* condition; // Expression node
    Block* then_block;
    Block* else_block; // optional
};

// Match Statement
struct MatchStmt {
    AstNode base;
    void* scrutinee; // Expression node
    void** patterns; // Pattern nodes
    Block** bodies; // Block nodes
    size_t case_count;
};

// Literal
struct Literal {
    AstNode base;
    enum { LIT_INT64, LIT_STRING, LIT_BOOL, LIT_UNIT } kind;
    union {
        int64_t int64_val;
        char* string_val;
        bool bool_val;
    } value;
};

// Identifier Expression
struct IdentifierExpr {
    AstNode base;
    char* name;
};

// Call Expression
struct CallExpr {
    AstNode base;
    void* function; // Expression node
    void** arguments; // Expression nodes
    size_t argument_count;
};

// Lambda Expression
struct LambdaExpr {
    AstNode base;
    char** param_names;
    size_t param_count;
    Block* body;
};

// If Expression
struct IfExpr {
    AstNode base;
    void* condition; // Expression node
    void* then_expr; // Expression node
    void* else_expr; // Expression node
};

// Match Expression
struct MatchExpr {
    AstNode base;
    void* scrutinee; // Expression node
    void** patterns; // Pattern nodes
    void** bodies; // Expression nodes
    size_t case_count;
};

// Pipe Expression
struct PipeExpr {
    AstNode base;
    void* left; // Expression node
    void* right; // Expression node
};

// Constructor Pattern
struct ConstructorPattern {
    AstNode base;
    char* constructor_name;
    void** fields; // Pattern nodes
    size_t field_count;
};

// Wildcard Pattern
struct WildcardPattern {
    AstNode base;
};

// Primitive Type
struct PrimitiveType {
    AstNode base;
    enum { PRIM_INT64, PRIM_STRING, PRIM_BOOL, PRIM_UNIT } kind;
};

// Named Type
struct NamedType {
    AstNode base;
    char* name;
};

// Generic Type
struct GenericType {
    AstNode base;
    char* name;
    void** type_args; // Type nodes
    size_t type_arg_count;
};

// Function Type
struct FunctionType {
    AstNode base;
    void** param_types; // Type nodes
    size_t param_count;
    void* return_type; // Type node
};

// AST construction functions
Program* ast_create_program(void);
void ast_free_program(Program* program);

// Utility functions
void ast_free_node(AstNode* node);
char* ast_node_to_string(AstNode* node);

#endif // MANAKNIGHT_AST_H