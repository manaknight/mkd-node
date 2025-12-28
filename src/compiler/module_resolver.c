#include "module_resolver.h"
#include "errors.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>

// Module resolver creation and destruction
ModuleResolver* create_module_resolver(const char* base_path) {
    ModuleResolver* resolver = malloc(sizeof(ModuleResolver));
    resolver->base_path = strdup(base_path ? base_path : ".");
    resolver->resolved_modules = NULL;
    resolver->resolved_count = 0;
    resolver->dep_graph = create_dependency_graph();
    return resolver;
}

void free_module_resolver(ModuleResolver* resolver) {
    if (!resolver) return;

    for (size_t i = 0; i < resolver->resolved_count; i++) {
        free(resolver->resolved_modules[i]->module_name);
        free(resolver->resolved_modules[i]->file_path);
        if (resolver->resolved_modules[i]->content) {
            free(resolver->resolved_modules[i]->content);
        }
        free(resolver->resolved_modules[i]->dependencies);
        free(resolver->resolved_modules[i]);
    }
    free(resolver->resolved_modules);
    free(resolver->base_path);
    free_dependency_graph(resolver->dep_graph);
    free(resolver);
}

// Convert module name to file path
char* module_name_to_path(const char* module_name, const char* base_path) {
    if (!module_name || !base_path) return NULL;

    // Convert dots to path separators: "auth.user" -> "auth/user.mk"
    size_t name_len = strlen(module_name);
    size_t base_len = strlen(base_path);

    // Count dots to determine path length
    size_t dot_count = 0;
    for (size_t i = 0; i < name_len; i++) {
        if (module_name[i] == '.') dot_count++;
    }

    // Allocate space: base + "/" + name with dots->"/" + ".mk" + null
    size_t path_len = base_len + 1 + name_len - dot_count + 3 + 1; // +3 for ".mk", +1 for null
    char* path = malloc(path_len);

    // Build path
    strcpy(path, base_path);
    strcat(path, "/");

    size_t path_pos = base_len + 1;
    for (size_t i = 0; i < name_len; i++) {
        if (module_name[i] == '.') {
            path[path_pos++] = '/';
        } else {
            path[path_pos++] = module_name[i];
        }
    }

    // Add .mk extension
    strcpy(path + path_pos, ".mk");

    return path;
}

// Module resolution
ResolvedModule* resolve_module(ModuleResolver* resolver, const char* module_name) {
    // Check if already resolved
    for (size_t i = 0; i < resolver->resolved_count; i++) {
        if (strcmp(resolver->resolved_modules[i]->module_name, module_name) == 0) {
            return resolver->resolved_modules[i];
        }
    }

    // Create new resolved module
    ResolvedModule* module = malloc(sizeof(ResolvedModule));
    module->module_name = strdup(module_name);
    module->file_path = module_name_to_path(module_name, resolver->base_path);
    module->exists = file_exists(module->file_path);
    module->content = NULL;
    module->content_length = 0;
    module->dependencies = NULL;
    module->dependency_count = 0;

    // Add to resolved modules array
    resolver->resolved_modules = realloc(resolver->resolved_modules,
                                        sizeof(ResolvedModule*) * (resolver->resolved_count + 1));
    resolver->resolved_modules[resolver->resolved_count++] = module;

    // Add to dependency graph
    add_module_to_graph(resolver->dep_graph, module_name);

    return module;
}

// Dependency management
bool add_dependency(ModuleResolver* resolver, const char* dependent, const char* dependency) {
    // Check for circular dependency
    if (has_circular_dependency(resolver, dependent)) {
        report_module_error(E5004_CIRCULAR_DEPENDENCY,
                           "Circular dependency detected",
                           NULL, 0, 0);
        return false;
    }

    // Add to dependency graph
    return add_dependency_to_graph(resolver->dep_graph, dependent, dependency);
}

bool has_circular_dependency(ModuleResolver* resolver, const char* module_name) {
    return detect_cycle(resolver->dep_graph, module_name);
}

// File operations
bool file_exists(const char* path) {
    struct stat st;
    return stat(path, &st) == 0 && S_ISREG(st.st_mode);
}

char* read_file_content(const char* path, uint32_t* length) {
    FILE* file = fopen(path, "rb");
    if (!file) {
        *length = 0;
        return NULL;
    }

    // Get file size
    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    if (file_size < 0) {
        fclose(file);
        *length = 0;
        return NULL;
    }

    // Read content
    char* content = malloc(file_size + 1);
    size_t bytes_read = fread(content, 1, file_size, file);
    content[bytes_read] = '\0';

    fclose(file);
    *length = (uint32_t)bytes_read;
    return content;
}

// Dependency graph implementation
DependencyGraph* create_dependency_graph() {
    DependencyGraph* graph = malloc(sizeof(DependencyGraph));
    graph->modules = NULL;
    graph->module_count = 0;
    graph->dependencies = NULL;
    return graph;
}

void free_dependency_graph(DependencyGraph* graph) {
    if (!graph) return;

    for (size_t i = 0; i < graph->module_count; i++) {
        free(graph->modules[i]);
        free(graph->dependencies[i]);
    }
    free(graph->modules);
    free(graph->dependencies);
    free(graph);
}

bool add_module_to_graph(DependencyGraph* graph, const char* module_name) {
    // Check if already exists
    for (size_t i = 0; i < graph->module_count; i++) {
        if (strcmp(graph->modules[i], module_name) == 0) {
            return true; // Already exists
        }
    }

    // Add new module
    graph->modules = realloc(graph->modules, sizeof(char*) * (graph->module_count + 1));
    graph->modules[graph->module_count] = strdup(module_name);

    // Resize dependency matrix
    graph->dependencies = realloc(graph->dependencies, sizeof(bool*) * (graph->module_count + 1));
    for (size_t i = 0; i <= graph->module_count; i++) {
        graph->dependencies[i] = realloc(graph->dependencies[i],
                                        sizeof(bool) * (graph->module_count + 1));
        // Initialize new row/column to false
        graph->dependencies[i][graph->module_count] = false;
        if (i < graph->module_count) {
            graph->dependencies[graph->module_count][i] = false;
        }
    }

    graph->module_count++;
    return true;
}

bool add_dependency_to_graph(DependencyGraph* graph, const char* from_module, const char* to_module) {
    // Find module indices
    size_t from_idx = SIZE_MAX;
    size_t to_idx = SIZE_MAX;

    for (size_t i = 0; i < graph->module_count; i++) {
        if (strcmp(graph->modules[i], from_module) == 0) from_idx = i;
        if (strcmp(graph->modules[i], to_module) == 0) to_idx = i;
    }

    if (from_idx == SIZE_MAX || to_idx == SIZE_MAX) {
        return false; // Module not found
    }

    graph->dependencies[from_idx][to_idx] = true;
    return true;
}

bool detect_cycle(DependencyGraph* graph, const char* start_module) {
    // Find start module index
    size_t start_idx = SIZE_MAX;
    for (size_t i = 0; i < graph->module_count; i++) {
        if (strcmp(graph->modules[i], start_module) == 0) {
            start_idx = i;
            break;
        }
    }

    if (start_idx == SIZE_MAX) return false;

    // Simple cycle detection using DFS
    bool* visited = calloc(graph->module_count, sizeof(bool));
    bool* rec_stack = calloc(graph->module_count, sizeof(bool));

    bool has_cycle = dfs_cycle_detect(graph, start_idx, visited, rec_stack);

    free(visited);
    free(rec_stack);

    return has_cycle;
}

static bool dfs_cycle_detect(DependencyGraph* graph, size_t node_idx,
                           bool* visited, bool* rec_stack) {
    visited[node_idx] = true;
    rec_stack[node_idx] = true;

    // Check all dependencies
    for (size_t i = 0; i < graph->module_count; i++) {
        if (graph->dependencies[node_idx][i]) { // node_idx depends on i
            if (!visited[i] && dfs_cycle_detect(graph, i, visited, rec_stack)) {
                return true;
            } else if (rec_stack[i]) {
                return true; // Cycle found
            }
        }
    }

    rec_stack[node_idx] = false;
    return false;
}

// Utility functions
void print_dependency_graph(DependencyGraph* graph) {
    printf("Dependency Graph:\n");
    printf("Modules: ");
    for (size_t i = 0; i < graph->module_count; i++) {
        printf("%s ", graph->modules[i]);
    }
    printf("\n");

    printf("Dependencies:\n");
    for (size_t i = 0; i < graph->module_count; i++) {
        printf("  %s -> ", graph->modules[i]);
        bool has_deps = false;
        for (size_t j = 0; j < graph->module_count; j++) {
            if (graph->dependencies[i][j]) {
                printf("%s ", graph->modules[j]);
                has_deps = true;
            }
        }
        if (!has_deps) printf("(none)");
        printf("\n");
    }
}

void print_resolved_modules(ModuleResolver* resolver) {
    printf("Resolved Modules:\n");
    for (size_t i = 0; i < resolver->resolved_count; i++) {
        ResolvedModule* mod = resolver->resolved_modules[i];
        printf("  %s -> %s (%s)\n",
               mod->module_name,
               mod->file_path,
               mod->exists ? "exists" : "not found");
    }
}
