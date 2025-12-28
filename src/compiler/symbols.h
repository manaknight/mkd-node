#ifndef SYMBOLS_H
#define SYMBOLS_H

#include "ast.h"
#include <stdbool.h>

// Forward declarations
typedef struct Symbol Symbol;
typedef struct Scope Scope;
typedef struct SymbolTable SymbolTable;

// Symbol kinds
typedef enum {
    SYMBOL_VARIABLE,
    SYMBOL_FUNCTION,
    SYMBOL_TYPE,
    SYMBOL_EFFECT,
    SYMBOL_MODULE,
} SymbolKind;

// Symbol structure
struct Symbol {
    char* name;
    SymbolKind kind;
    Type* type;           // For variables and functions
    void* declaration;    // Pointer to AST declaration node
    bool is_mutable;      // For variables
    Scope* defined_in;    // Scope where this symbol was defined
};

// Scope structure
struct Scope {
    Scope* parent;        // Parent scope (NULL for global)
    Symbol** symbols;     // Array of symbols in this scope
    size_t symbol_count;
    size_t symbol_capacity;
    char* scope_name;     // For debugging (e.g., "function foo", "block")
};

// Symbol table structure
struct SymbolTable {
    Scope* global_scope;
    Scope* current_scope;
    bool allow_shadows;   // Should be false for Manaknight
};

// Function declarations
SymbolTable* create_symbol_table();
void free_symbol_table(SymbolTable* table);

// Scope management
Scope* create_scope(Scope* parent, const char* name);
void free_scope(Scope* scope);
void enter_scope(SymbolTable* table, const char* name);
void exit_scope(SymbolTable* table);

// Symbol management
Symbol* create_symbol(const char* name, SymbolKind kind, Type* type, void* declaration);
void free_symbol(Symbol* symbol);
bool declare_symbol(SymbolTable* table, Symbol* symbol);
Symbol* resolve_symbol(SymbolTable* table, const char* name);
bool symbol_exists_in_current_scope(SymbolTable* table, const char* name);

// Prelude management
void load_prelude(SymbolTable* table);

// Utility functions
void print_symbol_table(SymbolTable* table);
void print_scope(Scope* scope, int indent);

#endif // SYMBOLS_H
