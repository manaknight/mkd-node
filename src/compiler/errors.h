#ifndef ERRORS_H
#define ERRORS_H

#include <stdint.h>

// Error code ranges as defined in the specification
#define ERROR_SYNTAX_MIN     1000
#define ERROR_SYNTAX_MAX     1999
#define ERROR_TYPE_MIN       2000
#define ERROR_TYPE_MAX       2999
#define ERROR_EFFECT_MIN     3000
#define ERROR_EFFECT_MAX     3999
#define ERROR_PATTERN_MIN    4000
#define ERROR_PATTERN_MAX    4999
#define ERROR_MODULE_MIN     5000
#define ERROR_MODULE_MAX     5999
#define ERROR_API_MIN        6000
#define ERROR_API_MAX        6999
#define ERROR_RUNTIME_MIN    7000
#define ERROR_RUNTIME_MAX    7999
#define ERROR_RESOURCE_MIN   8000
#define ERROR_RESOURCE_MAX   8999
#define ERROR_INTERNAL_MIN   9000
#define ERROR_INTERNAL_MAX   9999

// Error categories
typedef enum {
    CATEGORY_SYNTAX,
    CATEGORY_TYPE,
    CATEGORY_EFFECT,
    CATEGORY_PATTERN,
    CATEGORY_MODULE,
    CATEGORY_API,
    CATEGORY_RUNTIME,
    CATEGORY_RESOURCE,
    CATEGORY_INTERNAL,
} ErrorCategory;

// Error structure
typedef struct {
    uint32_t code;
    ErrorCategory category;
    const char* message;
    const char* file;
    uint32_t line;
    uint32_t column;
} CompilerError;

// Error code enums

// Syntax & Parsing Errors (E1000-E1999)
typedef enum {
    E1001_UNEXPECTED_TOKEN = 1001,
    E1002_MISSING_CLOSING_BRACE = 1002,
    E1003_INVALID_FUNCTION_DECLARATION = 1003,
    E1004_INVALID_API_DECLARATION = 1004,
    E1005_INVALID_TYPE_DECLARATION = 1005,
    E1006_EMPTY_BLOCK_NOT_ALLOWED = 1006,
} SyntaxError;

// Type System Errors (E2000-E2999)
typedef enum {
    E2001_UNKNOWN_IDENTIFIER = 2001,
    E2002_TYPE_MISMATCH = 2002,
    E2003_INVALID_FUNCTION_CALL = 2003,
    E2004_INVALID_RETURN_TYPE = 2004,
    E2005_MISSING_RETURN_VALUE = 2005,
    E2006_REASSIGNMENT_FORBIDDEN = 2006,
    E2007_INVALID_CONDITION_TYPE = 2007,
} TypeError;

// Effect System Errors (E3000-E3999)
typedef enum {
    E3001_UNDECLARED_EFFECT_USAGE = 3001,
    E3002_EFFECT_LEAKAGE = 3002,
    E3003_EFFECT_ESCALATION = 3003,
    E3004_EFFECT_USAGE_IN_LAMBDA = 3004,
    E3005_INVALID_EFFECT_DECLARATION = 3005,
} EffectError;

// Pattern Matching Errors (E4000-E4999)
typedef enum {
    E4001_NON_EXHAUSTIVE_MATCH = 4001,
    E4002_INVALID_MATCH_TARGET = 4002,
    E4003_DUPLICATE_PATTERN = 4003,
    E4004_INCONSISTENT_MATCH_RESULT_TYPES = 4004,
} PatternError;

// Module & Import Errors (E5000-E5999)
typedef enum {
    E5001_MODULE_NOT_FOUND = 5001,
    E5002_DUPLICATE_MODULE_DEFINITION = 5002,
    E5003_SYMBOL_NOT_EXPORTED = 5003,
    E5004_CIRCULAR_DEPENDENCY = 5004,
} ModuleError;

// API Definition Errors (E6000-E6999)
typedef enum {
    E6001_INVALID_HTTP_METHOD = 6001,
    E6002_INVALID_ROUTE_PATH = 6002,
    E6003_MISSING_API_RESPONSE = 6003,
    E6004_INVALID_API_PARAMETER_TYPE = 6004,
    E6005_UNDECLARED_EFFECT_IN_API = 6005,
} ApiError;

// Runtime Errors (E7000-E7999)
typedef enum {
    E7001_INVALID_RESULT_VALUE = 7001,
    E7002_INVALID_OPTION_VALUE = 7002,
    E7003_SERIALIZATION_FAILURE = 7003,
    E7004_INVALID_BYTECODE = 7004,
} RuntimeError;

// Resource Limit Errors (E8000-E8999)
typedef enum {
    E8001_EXECUTION_TIMEOUT = 8001,
    E8002_MEMORY_LIMIT_EXCEEDED = 8002,
    E8003_RECURSION_LIMIT_EXCEEDED = 8003,
    E8004_ALLOCATION_LIMIT_EXCEEDED = 8004,
} ResourceError;

// Internal Errors (E9000-E9999)
typedef enum {
    E9001_COMPILER_INTERNAL_ERROR = 9001,
    E9002_VM_INTERNAL_ERROR = 9002,
} InternalError;

// Function declarations
CompilerError* create_error(uint32_t code, const char* message,
                           const char* file, uint32_t line, uint32_t column);
void free_error(CompilerError* error);
const char* get_error_category_name(ErrorCategory category);
const char* get_error_message(uint32_t code);

// Error reporting functions
void report_error(CompilerError* error);
void report_syntax_error(SyntaxError code, const char* message,
                        const char* file, uint32_t line, uint32_t column);
void report_type_error(TypeError code, const char* message,
                      const char* file, uint32_t line, uint32_t column);
void report_effect_error(EffectError code, const char* message,
                        const char* file, uint32_t line, uint32_t column);
void report_pattern_error(PatternError code, const char* message,
                         const char* file, uint32_t line, uint32_t column);
void report_module_error(ModuleError code, const char* message,
                        const char* file, uint32_t line, uint32_t column);
void report_api_error(ApiError code, const char* message,
                     const char* file, uint32_t line, uint32_t column);
void report_runtime_error(RuntimeError code, const char* message,
                         const char* file, uint32_t line, uint32_t column);
void report_resource_error(ResourceError code, const char* message,
                          const char* file, uint32_t line, uint32_t column);
void report_internal_error(InternalError code, const char* message,
                          const char* file, uint32_t line, uint32_t column);

#endif // ERRORS_H
