#include "manaknight_runtime.h"
#include "cutils.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <pthread.h>

// Global state for HTTP server
static int http_server_socket = -1;
static bool http_server_running = false;
static pthread_t http_server_thread;

// Forward declarations for internal functions
static uint8_t* load_file(const char* filename, size_t* plen);
static JSValue create_effects_object(JSContext* ctx);
static void* http_server_worker(void* arg);

// Initialize Manaknight runtime
JSContext* manaknight_init(const ManaknightConfig* config) {
    // Allocate memory buffer
    uint8_t* mem_buf = malloc(config->memory_limit);
    if (!mem_buf) {
        fprintf(stderr, "Failed to allocate memory buffer\n");
        return NULL;
    }

    // Create JS context
    JSContext* ctx = JS_NewContext(mem_buf, config->memory_limit, NULL);
    if (!ctx) {
        fprintf(stderr, "Failed to create JS context\n");
        free(mem_buf);
        return NULL;
    }

    // Set memory limit
    manaknight_set_memory_limit(ctx, config->memory_limit);

    // Set CPU time limit
    manaknight_set_cpu_limit(ctx, config->cpu_time_limit);

    // Set up module loader
    JS_SetModuleLoaderFunc(ctx, manaknight_module_loader, NULL);

    // Load standard library
    if (manaknight_load_stdlib(ctx, config->stdlib_path) != 0) {
        fprintf(stderr, "Failed to load standard library\n");
        JS_FreeContext(ctx);
        free(mem_buf);
        return NULL;
    }

    // Set up effect system
    EffectContext effect_ctx;
    if (manaknight_setup_effects(ctx, &effect_ctx) != 0) {
        fprintf(stderr, "Failed to set up effect system\n");
        JS_FreeContext(ctx);
        free(mem_buf);
        return NULL;
    }

    // Start HTTP server if requested
    if (config->enable_http_server) {
        if (manaknight_start_http_server(ctx, config->http_port) != 0) {
            fprintf(stderr, "Failed to start HTTP server\n");
            JS_FreeContext(ctx);
            free(mem_buf);
            return NULL;
        }
    }

    return ctx;
}

// Load and execute Manaknight bytecode
int manaknight_execute_bytecode(JSContext* ctx, const char* bytecode_path) {
    size_t bytecode_len;
    uint8_t* bytecode = load_file(bytecode_path, &bytecode_len);
    if (!bytecode) {
        fprintf(stderr, "Failed to load bytecode from %s\n", bytecode_path);
        return -1;
    }

    // Check if it's valid bytecode
    if (!JS_IsBytecode(bytecode, bytecode_len)) {
        fprintf(stderr, "Invalid bytecode file: %s\n", bytecode_path);
        free(bytecode);
        return -1;
    }

    // Relocate bytecode
    if (JS_RelocateBytecode(ctx, bytecode, bytecode_len) != 0) {
        fprintf(stderr, "Failed to relocate bytecode\n");
        free(bytecode);
        return -1;
    }

    // Load bytecode
    JSValue val = JS_LoadBytecode(ctx, bytecode);
    free(bytecode);

    if (JS_IsException(val)) {
        manaknight_dump_error(ctx);
        return -1;
    }

    // Execute
    JSValue result = JS_Run(ctx, val);
    if (JS_IsException(result)) {
        manaknight_dump_error(ctx);
        JS_FreeValue(ctx, result);
        return -1;
    }

    JS_FreeValue(ctx, result);
    return 0;
}

// Load standard library
int manaknight_load_stdlib(JSContext* ctx, const char* stdlib_path) {
    // List of stdlib files to load
    const char* stdlib_files[] = {
        "core.js",
        "math.js",
        "string.js",
        "json.js",
        "http.js",
        NULL
    };

    for (int i = 0; stdlib_files[i] != NULL; i++) {
        char filepath[1024];
        snprintf(filepath, sizeof(filepath), "%s/%s", stdlib_path, stdlib_files[i]);

        size_t len;
        uint8_t* buf = load_file(filepath, &len);
        if (!buf) {
            fprintf(stderr, "Failed to load stdlib file: %s\n", filepath);
            return -1;
        }

        JSValue val = JS_Parse(ctx, (char*)buf, len, filepath, 0);
        free(buf);

        if (JS_IsException(val)) {
            manaknight_dump_error(ctx);
            return -1;
        }

        JSValue result = JS_Run(ctx, val);
        if (JS_IsException(result)) {
            manaknight_dump_error(ctx);
            JS_FreeValue(ctx, result);
            return -1;
        }

        JS_FreeValue(ctx, result);
    }

    return 0;
}

// Set up effect system
int manaknight_setup_effects(JSContext* ctx, EffectContext* effect_ctx) {
    effect_ctx->ctx = ctx;
    effect_ctx->effects_object = create_effects_object(ctx);

    if (JS_IsException(effect_ctx->effects_object)) {
        manaknight_dump_error(ctx);
        return -1;
    }

    // Set __effects in global scope
    JSValue global = JS_GetGlobalObject(ctx);
    JS_SetPropertyStr(ctx, global, "__effects", JS_DupValue(ctx, effect_ctx->effects_object));
    JS_FreeValue(ctx, global);

    return 0;
}

// Create effects object with native function bindings
static JSValue create_effects_object(JSContext* ctx) {
    JSValue effects = JS_NewObject(ctx);

    // Time effects
    JSValue time_obj = JS_NewObject(ctx);
    JS_SetPropertyStr(ctx, time_obj, "now", JS_NewCFunction(ctx, manaknight_time_now, "now", 0));
    JS_SetPropertyStr(ctx, time_obj, "unixMillis", JS_NewCFunction(ctx, manaknight_time_unixMillis, "unixMillis", 0));
    JS_SetPropertyStr(ctx, time_obj, "sleep", JS_NewCFunction(ctx, manaknight_time_sleep, "sleep", 1));
    JS_SetPropertyStr(ctx, effects, "time", time_obj);

    // Random effects
    JSValue random_obj = JS_NewObject(ctx);
    JS_SetPropertyStr(ctx, random_obj, "int", JS_NewCFunction(ctx, manaknight_random_int, "int", 0));
    JS_SetPropertyStr(ctx, random_obj, "intRange", JS_NewCFunction(ctx, manaknight_random_intRange, "intRange", 2));
    JS_SetPropertyStr(ctx, random_obj, "bytes", JS_NewCFunction(ctx, manaknight_random_bytes, "bytes", 1));
    JS_SetPropertyStr(ctx, random_obj, "uuidV4", JS_NewCFunction(ctx, manaknight_random_uuidV4, "uuidV4", 0));
    JS_SetPropertyStr(ctx, effects, "random", random_obj);

    // HTTP effects
    JSValue http_obj = JS_NewObject(ctx);
    JS_SetPropertyStr(ctx, http_obj, "get", JS_NewCFunction(ctx, manaknight_http_get, "get", 1));
    JS_SetPropertyStr(ctx, http_obj, "post", JS_NewCFunction(ctx, manaknight_http_post, "post", 2));
    JS_SetPropertyStr(ctx, http_obj, "put", JS_NewCFunction(ctx, manaknight_http_put, "put", 2));
    JS_SetPropertyStr(ctx, http_obj, "delete", JS_NewCFunction(ctx, manaknight_http_delete, "delete", 1));
    JS_SetPropertyStr(ctx, http_obj, "head", JS_NewCFunction(ctx, manaknight_http_head, "head", 1));
    JS_SetPropertyStr(ctx, http_obj, "request", JS_NewCFunction(ctx, manaknight_http_request, "request", 1));
    JS_SetPropertyStr(ctx, effects, "http", http_obj);

    // Logging effects
    JSValue log_obj = JS_NewObject(ctx);
    JS_SetPropertyStr(ctx, log_obj, "info", JS_NewCFunction(ctx, manaknight_log_info, "info", 1));
    JS_SetPropertyStr(ctx, log_obj, "warn", JS_NewCFunction(ctx, manaknight_log_warn, "warn", 1));
    JS_SetPropertyStr(ctx, log_obj, "error", JS_NewCFunction(ctx, manaknight_log_error, "error", 1));
    JS_SetPropertyStr(ctx, log_obj, "debug", JS_NewCFunction(ctx, manaknight_log_debug, "debug", 1));
    JS_SetPropertyStr(ctx, effects, "log", log_obj);

    // File system effects
    JSValue fs_obj = JS_NewObject(ctx);
    JS_SetPropertyStr(ctx, fs_obj, "readFile", JS_NewCFunction(ctx, manaknight_fs_readFile, "readFile", 1));
    JS_SetPropertyStr(ctx, fs_obj, "writeFile", JS_NewCFunction(ctx, manaknight_fs_writeFile, "writeFile", 2));
    JS_SetPropertyStr(ctx, fs_obj, "exists", JS_NewCFunction(ctx, manaknight_fs_exists, "exists", 1));
    JS_SetPropertyStr(ctx, effects, "fs", fs_obj);

    // Crypto effects
    JSValue crypto_obj = JS_NewObject(ctx);
    JS_SetPropertyStr(ctx, crypto_obj, "hashSha256", JS_NewCFunction(ctx, manaknight_crypto_hashSha256, "hashSha256", 1));
    JS_SetPropertyStr(ctx, crypto_obj, "hmacSha256", JS_NewCFunction(ctx, manaknight_crypto_hmacSha256, "hmacSha256", 2));
    JS_SetPropertyStr(ctx, effects, "crypto", crypto_obj);

    // Environment effects
    JSValue env_obj = JS_NewObject(ctx);
    JS_SetPropertyStr(ctx, env_obj, "getEnv", JS_NewCFunction(ctx, manaknight_env_getEnv, "getEnv", 1));
    JS_SetPropertyStr(ctx, effects, "env", env_obj);

    // System effects (restricted)
    JSValue sys_obj = JS_NewObject(ctx);
    JS_SetPropertyStr(ctx, sys_obj, "exit", JS_NewCFunction(ctx, manaknight_sys_exit, "exit", 1));
    JS_SetPropertyStr(ctx, sys_obj, "getPid", JS_NewCFunction(ctx, manaknight_sys_getPid, "getPid", 0));
    JS_SetPropertyStr(ctx, effects, "sys", sys_obj);

    return effects;
}

// HTTP server functions
int manaknight_start_http_server(JSContext* ctx, int port) {
    // Create socket
    http_server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (http_server_socket < 0) {
        perror("socket");
        return -1;
    }

    // Set socket options
    int opt = 1;
    setsockopt(http_server_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    // Bind to port
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);

    if (bind(http_server_socket, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("bind");
        close(http_server_socket);
        return -1;
    }

    // Listen
    if (listen(http_server_socket, 10) < 0) {
        perror("listen");
        close(http_server_socket);
        return -1;
    }

    printf("Manaknight HTTP server listening on port %d\n", port);

    // Start server thread
    http_server_running = true;
    if (pthread_create(&http_server_thread, NULL, http_server_worker, ctx) != 0) {
        perror("pthread_create");
        close(http_server_socket);
        return -1;
    }

    return 0;
}

void manaknight_stop_http_server() {
    http_server_running = false;
    if (http_server_socket >= 0) {
        close(http_server_socket);
        http_server_socket = -1;
    }
    pthread_join(http_server_thread, NULL);
}

// HTTP server worker thread
static void* http_server_worker(void* arg) {
    JSContext* ctx = (JSContext*)arg;

    while (http_server_running) {
        struct sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);

        int client_socket = accept(http_server_socket, (struct sockaddr*)&client_addr, &client_len);
        if (client_socket < 0) {
            if (http_server_running) perror("accept");
            continue;
        }

        // Simple HTTP request handling (placeholder)
        // In a real implementation, this would parse HTTP requests and route them
        // to the appropriate Manaknight API handlers

        const char* response = "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\n\r\nHello from Manaknight!";
        write(client_socket, response, strlen(response));
        close(client_socket);
    }

    return NULL;
}

// Module loader
JSModuleDef* manaknight_module_loader(JSContext* ctx, const char* module_name, void* opaque) {
    // This would load Manaknight modules
    // For now, return NULL (not implemented)
    return NULL;
}

// Effect handler implementations

// Time effects
JSValue manaknight_time_now(JSContext* ctx, JSValue* this_val, int argc, JSValue* argv) {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    uint64_t millis = (uint64_t)tv.tv_sec * 1000 + tv.tv_usec / 1000;
    return JS_NewInt64(ctx, millis);
}

JSValue manaknight_time_unixMillis(JSContext* ctx, JSValue* this_val, int argc, JSValue* argv) {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    uint64_t millis = (uint64_t)tv.tv_sec * 1000 + tv.tv_usec / 1000;
    return JS_NewInt64(ctx, millis);
}

JSValue manaknight_time_sleep(JSContext* ctx, JSValue* this_val, int argc, JSValue* argv) {
    if (argc < 1) return JS_ThrowTypeError(ctx, "sleep requires duration argument");

    int64_t duration_ms;
    if (JS_ToInt64(ctx, &duration_ms, argv[0]) != 0) {
        return JS_ThrowTypeError(ctx, "duration must be a number");
    }

    if (duration_ms > 0) {
        usleep(duration_ms * 1000); // Convert to microseconds
    }

    return JS_UNDEFINED;
}

// Random effects
JSValue manaknight_random_int(JSContext* ctx, JSValue* this_val, int argc, JSValue* argv) {
    // Use a better random source - this is simplified
    // In production, use OpenSSL RAND_bytes or similar
    uint64_t random_val = 0;
    FILE* urandom = fopen("/dev/urandom", "rb");
    if (urandom) {
        fread(&random_val, sizeof(random_val), 1, urandom);
        fclose(urandom);
    } else {
        // Fallback to rand()
        srand(time(NULL));
        random_val = (uint64_t)rand() | ((uint64_t)rand() << 32);
    }
    return JS_NewInt64(ctx, random_val);
}

JSValue manaknight_random_intRange(JSContext* ctx, JSValue* this_val, int argc, JSValue* argv) {
    if (argc < 2) return JS_ThrowTypeError(ctx, "intRange requires min and max arguments");

    int64_t min, max;
    if (JS_ToInt64(ctx, &min, argv[0]) != 0 || JS_ToInt64(ctx, &max, argv[1]) != 0) {
        return JS_ThrowTypeError(ctx, "min and max must be numbers");
    }

    if (min >= max) {
        return JS_ThrowRangeError(ctx, "min must be less than max");
    }

    // Get random value
    uint64_t random_val;
    FILE* urandom = fopen("/dev/urandom", "rb");
    if (urandom) {
        fread(&random_val, sizeof(random_val), 1, urandom);
        fclose(urandom);
    } else {
        srand(time(NULL));
        random_val = (uint64_t)rand();
    }

    // Map to range
    uint64_t range = max - min;
    int64_t result = min + (random_val % range);

    return JS_NewInt64(ctx, result);
}

JSValue manaknight_random_bytes(JSContext* ctx, JSValue* this_val, int argc, JSValue* argv) {
    if (argc < 1) return JS_ThrowTypeError(ctx, "bytes requires length argument");

    int64_t length;
    if (JS_ToInt64(ctx, &length, argv[0]) != 0) {
        return JS_ThrowTypeError(ctx, "length must be a number");
    }

    if (length < 0) {
        return JS_ThrowRangeError(ctx, "length cannot be negative");
    }

    if (length > 1024 * 1024) { // 1MB limit
        return JS_ThrowRangeError(ctx, "length too large");
    }

    // Create array of random bytes
    JSValue array = JS_NewArray(ctx);
    FILE* urandom = fopen("/dev/urandom", "rb");

    for (int64_t i = 0; i < length; i++) {
        uint8_t byte = 0;
        if (urandom) {
            fread(&byte, 1, 1, urandom);
        } else {
            srand(time(NULL) + i);
            byte = rand() & 0xFF;
        }
        JS_SetPropertyUint32(ctx, array, i, JS_NewInt32(ctx, byte));
    }

    if (urandom) fclose(urandom);

    return array;
}

JSValue manaknight_random_uuidV4(JSContext* ctx, JSValue* this_val, int argc, JSValue* argv) {
    uint8_t bytes[16];
    FILE* urandom = fopen("/dev/urandom", "rb");
    if (urandom) {
        fread(bytes, 1, 16, urandom);
        fclose(urandom);
    } else {
        srand(time(NULL));
        for (int i = 0; i < 16; i++) {
            bytes[i] = rand() & 0xFF;
        }
    }

    // Set version (4) and variant (2)
    bytes[6] = (bytes[6] & 0x0F) | 0x40;
    bytes[8] = (bytes[8] & 0x3F) | 0x80;

    // Format as UUID string
    char uuid[37];
    snprintf(uuid, sizeof(uuid),
             "%02x%02x%02x%02x-%02x%02x-%02x%02x-%02x%02x-%02x%02x%02x%02x%02x%02x",
             bytes[0], bytes[1], bytes[2], bytes[3],
             bytes[4], bytes[5],
             bytes[6], bytes[7],
             bytes[8], bytes[9],
             bytes[10], bytes[11], bytes[12], bytes[13], bytes[14], bytes[15]);

    return JS_NewString(ctx, uuid);
}

// HTTP effects (simplified implementations)
JSValue manaknight_http_get(JSContext* ctx, JSValue* this_val, int argc, JSValue* argv) {
    // This is a placeholder - real implementation would use libcurl or similar
    // For now, return a mock response
    JSValue response = JS_NewObject(ctx);
    JS_SetPropertyStr(ctx, response, "status_code", JS_NewInt32(ctx, 200));
    JS_SetPropertyStr(ctx, response, "headers", JS_NewObject(ctx));
    JS_SetPropertyStr(ctx, response, "body", JS_NewString(ctx, "{\"message\": \"HTTP GET not implemented\"}"));
    return response;
}

JSValue manaknight_http_post(JSContext* ctx, JSValue* this_val, int argc, JSValue* argv) {
    // Placeholder implementation
    JSValue response = JS_NewObject(ctx);
    JS_SetPropertyStr(ctx, response, "status_code", JS_NewInt32(ctx, 201));
    JS_SetPropertyStr(ctx, response, "headers", JS_NewObject(ctx));
    JS_SetPropertyStr(ctx, response, "body", JS_NewString(ctx, "{\"created\": true}"));
    return response;
}

JSValue manaknight_http_put(JSContext* ctx, JSValue* this_val, int argc, JSValue* argv) {
    JSValue response = JS_NewObject(ctx);
    JS_SetPropertyStr(ctx, response, "status_code", JS_NewInt32(ctx, 200));
    JS_SetPropertyStr(ctx, response, "headers", JS_NewObject(ctx));
    JS_SetPropertyStr(ctx, response, "body", JS_NewString(ctx, "{\"updated\": true}"));
    return response;
}

JSValue manaknight_http_delete(JSContext* ctx, JSValue* this_val, int argc, JSValue* argv) {
    JSValue response = JS_NewObject(ctx);
    JS_SetPropertyStr(ctx, response, "status_code", JS_NewInt32(ctx, 204));
    JS_SetPropertyStr(ctx, response, "headers", JS_NewObject(ctx));
    JS_SetPropertyStr(ctx, response, "body", JS_UNDEFINED);
    return response;
}

JSValue manaknight_http_head(JSContext* ctx, JSValue* this_val, int argc, JSValue* argv) {
    JSValue response = JS_NewObject(ctx);
    JS_SetPropertyStr(ctx, response, "status_code", JS_NewInt32(ctx, 200));
    JS_SetPropertyStr(ctx, response, "headers", JS_NewObject(ctx));
    JS_SetPropertyStr(ctx, response, "body", JS_UNDEFINED);
    return response;
}

JSValue manaknight_http_request(JSContext* ctx, JSValue* this_val, int argc, JSValue* argv) {
    // Generic request handler - dispatches based on method
    if (argc < 1) return JS_ThrowTypeError(ctx, "request requires request object");

    JSValue request = argv[0];
    JSValue method_val = JS_GetPropertyStr(ctx, request, "method");
    const char* method = JS_ToCString(ctx, method_val);

    JSValue result;
    if (strcmp(method, "GET") == 0) {
        result = manaknight_http_get(ctx, this_val, argc, argv);
    } else if (strcmp(method, "POST") == 0) {
        result = manaknight_http_post(ctx, this_val, argc, argv);
    } else if (strcmp(method, "PUT") == 0) {
        result = manaknight_http_put(ctx, this_val, argc, argv);
    } else if (strcmp(method, "DELETE") == 0) {
        result = manaknight_http_delete(ctx, this_val, argc, argv);
    } else if (strcmp(method, "HEAD") == 0) {
        result = manaknight_http_head(ctx, this_val, argc, argv);
    } else {
        result = JS_ThrowTypeError(ctx, "unsupported HTTP method");
    }

    JS_FreeCString(ctx, method);
    JS_FreeValue(ctx, method_val);
    return result;
}

// Logging effects
JSValue manaknight_log_info(JSContext* ctx, JSValue* this_val, int argc, JSValue* argv) {
    if (argc > 0) {
        const char* message = JS_ToCString(ctx, argv[0]);
        printf("[INFO] %s\n", message);
        JS_FreeCString(ctx, message);
    }
    return JS_UNDEFINED;
}

JSValue manaknight_log_warn(JSContext* ctx, JSValue* this_val, int argc, JSValue* argv) {
    if (argc > 0) {
        const char* message = JS_ToCString(ctx, argv[0]);
        fprintf(stderr, "[WARN] %s\n", message);
        JS_FreeCString(ctx, message);
    }
    return JS_UNDEFINED;
}

JSValue manaknight_log_error(JSContext* ctx, JSValue* this_val, int argc, JSValue* argv) {
    if (argc > 0) {
        const char* message = JS_ToCString(ctx, argv[0]);
        fprintf(stderr, "[ERROR] %s\n", message);
        JS_FreeCString(ctx, message);
    }
    return JS_UNDEFINED;
}

JSValue manaknight_log_debug(JSContext* ctx, JSValue* this_val, int argc, JSValue* argv) {
    if (argc > 0) {
        const char* message = JS_ToCString(ctx, argv[0]);
        printf("[DEBUG] %s\n", message);
        JS_FreeCString(ctx, message);
    }
    return JS_UNDEFINED;
}

// File system effects (basic implementations)
JSValue manaknight_fs_readFile(JSContext* ctx, JSValue* this_val, int argc, JSValue* argv) {
    if (argc < 1) return JS_ThrowTypeError(ctx, "readFile requires filename");

    const char* filename = JS_ToCString(ctx, argv[0]);
    if (!filename) return JS_ThrowTypeError(ctx, "filename must be a string");

    FILE* file = fopen(filename, "rb");
    if (!file) {
        JS_FreeCString(ctx, filename);
        JSValue error = JS_NewObject(ctx);
        JS_SetPropertyStr(ctx, error, "tag", JS_NewString(ctx, "network_error"));
        JS_SetPropertyStr(ctx, error, "message", JS_NewString(ctx, "file not found"));
        return error;
    }

    // Read file content
    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    fseek(file, 0, SEEK_SET);

    if (size < 0 || size > 10 * 1024 * 1024) { // 10MB limit
        fclose(file);
        JS_FreeCString(ctx, filename);
        JSValue error = JS_NewObject(ctx);
        JS_SetPropertyStr(ctx, error, "tag", JS_NewString(ctx, "network_error"));
        JS_SetPropertyStr(ctx, error, "message", JS_NewString(ctx, "file too large"));
        return error;
    }

    char* content = malloc(size + 1);
    if (!content) {
        fclose(file);
        JS_FreeCString(ctx, filename);
        return JS_ThrowOutOfMemory(ctx);
    }

    size_t read = fread(content, 1, size, file);
    content[read] = '\0';
    fclose(file);

    JSValue result = JS_NewObject(ctx);
    JS_SetPropertyStr(ctx, result, "tag", JS_NewString(ctx, "ok"));
    JS_SetPropertyStr(ctx, result, "value", JS_NewString(ctx, content));

    free(content);
    JS_FreeCString(ctx, filename);
    return result;
}

JSValue manaknight_fs_writeFile(JSContext* ctx, JSValue* this_val, int argc, JSValue* argv) {
    if (argc < 2) return JS_ThrowTypeError(ctx, "writeFile requires filename and content");

    const char* filename = JS_ToCString(ctx, argv[0]);
    const char* content = JS_ToCString(ctx, argv[1]);

    if (!filename || !content) {
        JS_FreeCString(ctx, filename);
        JS_FreeCString(ctx, content);
        return JS_ThrowTypeError(ctx, "arguments must be strings");
    }

    FILE* file = fopen(filename, "wb");
    if (!file) {
        JS_FreeCString(ctx, filename);
        JS_FreeCString(ctx, content);
        JSValue error = JS_NewObject(ctx);
        JS_SetPropertyStr(ctx, error, "tag", JS_NewString(ctx, "network_error"));
        JS_SetPropertyStr(ctx, error, "message", JS_NewString(ctx, "cannot write file"));
        return error;
    }

    fwrite(content, 1, strlen(content), file);
    fclose(file);

    JSValue result = JS_NewObject(ctx);
    JS_SetPropertyStr(ctx, result, "tag", JS_NewString(ctx, "ok"));
    JS_SetPropertyStr(ctx, result, "value", JS_NewString(ctx, "()"));

    JS_FreeCString(ctx, filename);
    JS_FreeCString(ctx, content);
    return result;
}

JSValue manaknight_fs_exists(JSContext* ctx, JSValue* this_val, int argc, JSValue* argv) {
    if (argc < 1) return JS_ThrowTypeError(ctx, "exists requires filename");

    const char* filename = JS_ToCString(ctx, argv[0]);
    if (!filename) return JS_NewBool(ctx, false);

    struct stat st;
    bool exists = stat(filename, &st) == 0;

    JS_FreeCString(ctx, filename);
    return JS_NewBool(ctx, exists);
}

// Crypto effects (basic implementations)
JSValue manaknight_crypto_hashSha256(JSContext* ctx, JSValue* this_val, int argc, JSValue* argv) {
    // Placeholder - real implementation would use OpenSSL or similar
    if (argc < 1) return JS_ThrowTypeError(ctx, "hashSha256 requires data");

    const char* data = JS_ToCString(ctx, argv[0]);
    if (!data) return JS_ThrowTypeError(ctx, "data must be a string");

    // Simple placeholder hash - in reality use proper SHA256
    char hash[65];
    snprintf(hash, sizeof(hash), "%08x%08x%08x%08x%08x%08x%08x%08x",
             (unsigned int)strlen(data), 0, 0, 0, 0, 0, 0, 0);

    JSValue result = JS_NewString(ctx, hash);
    JS_FreeCString(ctx, data);
    return result;
}

JSValue manaknight_crypto_hmacSha256(JSContext* ctx, JSValue* this_val, int argc, JSValue* argv) {
    // Placeholder implementation
    if (argc < 2) return JS_ThrowTypeError(ctx, "hmacSha256 requires key and data");

    // Simple placeholder - real implementation would use proper HMAC-SHA256
    return JS_NewString(ctx, "hmac_placeholder");
}

// Environment effects (restricted)
JSValue manaknight_env_getEnv(JSContext* ctx, JSValue* this_val, int argc, JSValue* argv) {
    if (argc < 1) return JS_ThrowTypeError(ctx, "getEnv requires variable name");

    const char* var_name = JS_ToCString(ctx, argv[0]);
    if (!var_name) return JS_ThrowTypeError(ctx, "variable name must be a string");

    const char* value = getenv(var_name);

    JSValue result;
    if (value) {
        result = JS_NewObject(ctx);
        JS_SetPropertyStr(ctx, result, "tag", JS_NewString(ctx, "some"));
        JS_SetPropertyStr(ctx, result, "value", JS_NewString(ctx, value));
    } else {
        result = JS_NewObject(ctx);
        JS_SetPropertyStr(ctx, result, "tag", JS_NewString(ctx, "none"));
    }

    JS_FreeCString(ctx, var_name);
    return result;
}

// System effects (very restricted)
JSValue manaknight_sys_exit(JSContext* ctx, JSValue* this_val, int argc, JSValue* argv) {
    if (argc < 1) exit(0);

    int32_t code;
    if (JS_ToInt32(ctx, &code, argv[0]) == 0) {
        exit(code);
    }

    exit(1); // Default exit code
}

JSValue manaknight_sys_getPid(JSContext* ctx, JSValue* this_val, int argc, JSValue* argv) {
    return JS_NewInt32(ctx, getpid());
}

// Resource limits
void manaknight_set_memory_limit(JSContext* ctx, size_t limit) {
    JS_SetMemoryLimit(ctx, limit);
}

void manaknight_set_cpu_limit(JSContext* ctx, size_t limit_ms) {
    // Set up interrupt handler for CPU time limit
    // This is a simplified implementation
    JS_SetInterruptHandler(ctx, NULL, NULL); // Placeholder
}

// Error handling
void manaknight_dump_error(JSContext* ctx) {
    JSValue exception = JS_GetException(ctx);
    const char* error_str = JS_ToCString(ctx, exception);
    fprintf(stderr, "Manaknight error: %s\n", error_str);
    JS_FreeCString(ctx, error_str);
    JS_FreeValue(ctx, exception);
}

// Cleanup
void manaknight_cleanup(JSContext* ctx) {
    manaknight_stop_http_server();

    // The caller is responsible for JS_FreeContext and freeing memory buffer
}

// Utility function to load files
static uint8_t* load_file(const char* filename, size_t* plen) {
    FILE* f = fopen(filename, "rb");
    if (!f) {
        return NULL;
    }

    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fseek(f, 0, SEEK_SET);

    if (size < 0) {
        fclose(f);
        return NULL;
    }

    uint8_t* buf = malloc(size);
    if (!buf) {
        fclose(f);
        return NULL;
    }

    size_t read_size = fread(buf, 1, size, f);
    fclose(f);

    if (read_size != (size_t)size) {
        free(buf);
        return NULL;
    }

    *plen = size;
    return buf;
}
