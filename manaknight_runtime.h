#ifndef MANAKNIGHT_RUNTIME_H
#define MANAKNIGHT_RUNTIME_H

#include "mquickjs.h"
#include <stdbool.h>

// Manaknight runtime configuration
typedef struct {
    const char* stdlib_path;      // Path to stdlib directory
    size_t memory_limit;          // Memory limit in bytes
    size_t cpu_time_limit;        // CPU time limit in milliseconds
    bool enable_http_server;     // Whether to start HTTP server
    int http_port;               // HTTP server port
} ManaknightConfig;

// Module loading context
typedef struct {
    JSContext* ctx;
    const char* base_path;
} ModuleLoaderContext;

// Effect injection context
typedef struct {
    JSContext* ctx;
    JSValue effects_object;
} EffectContext;

// Function declarations

// Initialize Manaknight runtime
JSContext* manaknight_init(const ManaknightConfig* config);

// Load and execute Manaknight bytecode
int manaknight_execute_bytecode(JSContext* ctx, const char* bytecode_path);

// Load standard library
int manaknight_load_stdlib(JSContext* ctx, const char* stdlib_path);

// Set up effect system
int manaknight_setup_effects(JSContext* ctx, EffectContext* effect_ctx);

// HTTP server functions (if enabled)
int manaknight_start_http_server(JSContext* ctx, int port);
void manaknight_stop_http_server();

// Module loading
JSModuleDef* manaknight_module_loader(JSContext* ctx, const char* module_name,
                                    void* opaque);

// Effect handlers
JSValue manaknight_time_now(JSContext* ctx, JSValue* this_val, int argc, JSValue* argv);
JSValue manaknight_time_unixMillis(JSContext* ctx, JSValue* this_val, int argc, JSValue* argv);
JSValue manaknight_random_int(JSContext* ctx, JSValue* this_val, int argc, JSValue* argv);
JSValue manaknight_http_get(JSContext* ctx, JSValue* this_val, int argc, JSValue* argv);
JSValue manaknight_http_post(JSContext* ctx, JSValue* this_val, int argc, JSValue* argv);
JSValue manaknight_log_info(JSContext* ctx, JSValue* this_val, int argc, JSValue* argv);

// Resource limits
void manaknight_set_memory_limit(JSContext* ctx, size_t limit);
void manaknight_set_cpu_limit(JSContext* ctx, size_t limit_ms);

// Error handling
void manaknight_dump_error(JSContext* ctx);

// Cleanup
void manaknight_cleanup(JSContext* ctx);

#endif // MANAKNIGHT_RUNTIME_H
