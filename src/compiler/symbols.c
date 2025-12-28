#include "symbols.h"
#include "errors.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// Symbol table creation and destruction
SymbolTable* create_symbol_table() {
    SymbolTable* table = malloc(sizeof(SymbolTable));
    table->global_scope = create_scope(NULL, "global");
    table->current_scope = table->global_scope;
    table->allow_shadows = false; // Manaknight forbids shadowing
    return table;
}

void free_symbol_table(SymbolTable* table) {
    if (!table) return;
    free_scope(table->global_scope);
    free(table);
}

// Scope management
Scope* create_scope(Scope* parent, const char* name) {
    Scope* scope = malloc(sizeof(Scope));
    scope->parent = parent;
    scope->symbols = NULL;
    scope->symbol_count = 0;
    scope->symbol_capacity = 0;
    scope->scope_name = name ? strdup(name) : NULL;
    return scope;
}

void free_scope(Scope* scope) {
    if (!scope) return;

    for (size_t i = 0; i < scope->symbol_count; i++) {
        free_symbol(scope->symbols[i]);
    }
    free(scope->symbols);
    free(scope->scope_name);
    free(scope);
}

void enter_scope(SymbolTable* table, const char* name) {
    Scope* new_scope = create_scope(table->current_scope, name);
    table->current_scope = new_scope;
}

void exit_scope(SymbolTable* table) {
    if (!table->current_scope || !table->current_scope->parent) {
        // Cannot exit global scope
        return;
    }

    Scope* old_scope = table->current_scope;
    table->current_scope = old_scope->parent;

    // Note: In a real implementation, you might want to keep scopes around
    // for analysis, but for now we'll free them
    free_scope(old_scope);
}

// Symbol management
Symbol* create_symbol(const char* name, SymbolKind kind, Type* type, void* declaration) {
    Symbol* symbol = malloc(sizeof(Symbol));
    symbol->name = strdup(name);
    symbol->kind = kind;
    symbol->type = type;
    symbol->declaration = declaration;
    symbol->is_mutable = false; // Manaknight variables are immutable by default
    symbol->defined_in = NULL; // Set when declared
    return symbol;
}

void free_symbol(Symbol* symbol) {
    if (!symbol) return;
    free(symbol->name);
    free(symbol);
}

bool declare_symbol(SymbolTable* table, Symbol* symbol) {
    // Check for existing symbol in current scope
    if (!table->allow_shadows && symbol_exists_in_current_scope(table, symbol->name)) {
        report_type_error(E2006_REASSIGNMENT_FORBIDDEN,
                         "Variable shadowing is not allowed",
                         NULL, 0, 0); // TODO: Add location info
        free_symbol(symbol);
        return false;
    }

    // Add to current scope
    Scope* scope = table->current_scope;

    // Resize array if needed
    if (scope->symbol_count >= scope->symbol_capacity) {
        scope->symbol_capacity = scope->symbol_capacity == 0 ? 8 : scope->symbol_capacity * 2;
        scope->symbols = realloc(scope->symbols, sizeof(Symbol*) * scope->symbol_capacity);
    }

    symbol->defined_in = scope;
    scope->symbols[scope->symbol_count++] = symbol;
    return true;
}

Symbol* resolve_symbol(SymbolTable* table, const char* name) {
    Scope* current = table->current_scope;

    // Search from current scope up to global scope
    while (current) {
        for (size_t i = 0; i < current->symbol_count; i++) {
            if (strcmp(current->symbols[i]->name, name) == 0) {
                return current->symbols[i];
            }
        }
        current = current->parent;
    }

    return NULL; // Not found
}

bool symbol_exists_in_current_scope(SymbolTable* table, const char* name) {
    Scope* scope = table->current_scope;

    for (size_t i = 0; i < scope->symbol_count; i++) {
        if (strcmp(scope->symbols[i]->name, name) == 0) {
            return true;
        }
    }

    return false;
}

// Prelude management
void load_prelude(SymbolTable* table) {
    // Save current scope
    Scope* saved_scope = table->current_scope;

    // Switch to global scope for prelude
    table->current_scope = table->global_scope;

    // Add core types to prelude
    // Option<T>
    Type* option_type = create_named_type("Option");
    Symbol* option_symbol = create_symbol("Option", SYMBOL_TYPE, option_type, NULL);
    declare_symbol(table, option_symbol);

    // Result<T, E>
    Type* result_type = create_named_type("Result");
    Symbol* result_symbol = create_symbol("Result", SYMBOL_TYPE, result_type, NULL);
    declare_symbol(table, result_symbol);

    // List<T>
    Type* list_type = create_named_type("List");
    Symbol* list_symbol = create_symbol("List", SYMBOL_TYPE, list_type, NULL);
    declare_symbol(table, list_symbol);

    // Map<K, V>
    Type* map_type = create_named_type("Map");
    Symbol* map_symbol = create_symbol("Map", SYMBOL_TYPE, map_type, NULL);
    declare_symbol(table, map_symbol);

    // Bool
    Type* bool_type = create_primitive_type(PRIM_BOOL);
    Symbol* bool_symbol = create_symbol("Bool", SYMBOL_TYPE, bool_type, NULL);
    declare_symbol(table, bool_symbol);

    // Int
    Type* int_type = create_primitive_type(PRIM_INT);
    Symbol* int_symbol = create_symbol("Int", SYMBOL_TYPE, int_type, NULL);
    declare_symbol(table, int_symbol);

    // String
    Type* string_type = create_primitive_type(PRIM_STRING);
    Symbol* string_symbol = create_symbol("String", SYMBOL_TYPE, string_type, NULL);
    declare_symbol(table, string_symbol);

    // Restore original scope
    table->current_scope = saved_scope;
}

// Utility functions for debugging
void print_symbol_table(SymbolTable* table) {
    printf("Symbol Table:\n");
    print_scope(table->global_scope, 0);
}

void print_scope(Scope* scope, int indent) {
    for (int i = 0; i < indent; i++) printf("  ");
    printf("Scope: %s\n", scope->scope_name ? scope->scope_name : "unnamed");

    for (size_t i = 0; i < scope->symbol_count; i++) {
        Symbol* symbol = scope->symbols[i];
        for (int j = 0; j < indent + 1; j++) printf("  ");
        printf("Symbol: %s (%d)\n", symbol->name, symbol->kind);
    }

    // Print child scopes if they exist
    // Note: This is simplified - in a real implementation you'd track child scopes
}
