#ifndef MODULE_RESOLVER_H
#define MODULE_RESOLVER_H

#include <stdbool.h>
#include <stdint.h>

// Forward declarations
typedef struct ModuleResolver ModuleResolver;
typedef struct ResolvedModule ResolvedModule;
typedef struct DependencyGraph DependencyGraph;

// Resolved module structure
struct ResolvedModule {
    char* module_name;     // e.g., "auth.user"
    char* file_path;       // e.g., "./auth/user.mk"
    bool exists;           // Whether the file exists
    char* content;         // File content (NULL if not loaded)
    uint32_t content_length;
    ResolvedModule** dependencies;  // Modules this one imports
    size_t dependency_count;
};

// Dependency graph for cycle detection
struct DependencyGraph {
    char** modules;        // Array of module names
    size_t module_count;
    bool** dependencies;   // Adjacency matrix: dependencies[i][j] means module i depends on module j
};

// Module resolver structure
struct ModuleResolver {
    char* base_path;       // Base directory for resolution (e.g., ".")
    ResolvedModule** resolved_modules;
    size_t resolved_count;
    DependencyGraph* dep_graph;
};

// Function declarations
ModuleResolver* create_module_resolver(const char* base_path);
void free_module_resolver(ModuleResolver* resolver);

// Module resolution
ResolvedModule* resolve_module(ModuleResolver* resolver, const char* module_name);
char* module_name_to_path(const char* module_name, const char* base_path);

// Dependency management
bool add_dependency(ModuleResolver* resolver, const char* dependent, const char* dependency);
bool has_circular_dependency(ModuleResolver* resolver, const char* module_name);

// File operations
bool file_exists(const char* path);
char* read_file_content(const char* path, uint32_t* length);

// Dependency graph
DependencyGraph* create_dependency_graph();
void free_dependency_graph(DependencyGraph* graph);
bool add_module_to_graph(DependencyGraph* graph, const char* module_name);
bool add_dependency_to_graph(DependencyGraph* graph, const char* from_module, const char* to_module);
bool detect_cycle(DependencyGraph* graph, const char* start_module);

// Utility functions
void print_dependency_graph(DependencyGraph* graph);
void print_resolved_modules(ModuleResolver* resolver);

#endif // MODULE_RESOLVER_H
