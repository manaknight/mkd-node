#include "errors.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// Error creation and destruction
CompilerError* create_error(uint32_t code, const char* message,
                           const char* file, uint32_t line, uint32_t column) {
    CompilerError* error = malloc(sizeof(CompilerError));
    error->code = code;
    error->message = strdup(message);
    error->file = file ? strdup(file) : NULL;
    error->line = line;
    error->column = column;

    // Determine category from code
    if (code >= ERROR_SYNTAX_MIN && code <= ERROR_SYNTAX_MAX) {
        error->category = CATEGORY_SYNTAX;
    } else if (code >= ERROR_TYPE_MIN && code <= ERROR_TYPE_MAX) {
        error->category = CATEGORY_TYPE;
    } else if (code >= ERROR_EFFECT_MIN && code <= ERROR_EFFECT_MAX) {
        error->category = CATEGORY_EFFECT;
    } else if (code >= ERROR_PATTERN_MIN && code <= ERROR_PATTERN_MAX) {
        error->category = CATEGORY_PATTERN;
    } else if (code >= ERROR_MODULE_MIN && code <= ERROR_MODULE_MAX) {
        error->category = CATEGORY_MODULE;
    } else if (code >= ERROR_API_MIN && code <= ERROR_API_MAX) {
        error->category = CATEGORY_API;
    } else if (code >= ERROR_RUNTIME_MIN && code <= ERROR_RUNTIME_MAX) {
        error->category = CATEGORY_RUNTIME;
    } else if (code >= ERROR_RESOURCE_MIN && code <= ERROR_RESOURCE_MAX) {
        error->category = CATEGORY_RESOURCE;
    } else if (code >= ERROR_INTERNAL_MIN && code <= ERROR_INTERNAL_MAX) {
        error->category = CATEGORY_INTERNAL;
    } else {
        error->category = CATEGORY_INTERNAL; // Default to internal for unknown codes
    }

    return error;
}

void free_error(CompilerError* error) {
    if (!error) return;
    free((void*)error->message);
    if (error->file) free((void*)error->file);
    free(error);
}

const char* get_error_category_name(ErrorCategory category) {
    switch (category) {
        case CATEGORY_SYNTAX: return "SyntaxError";
        case CATEGORY_TYPE: return "TypeError";
        case CATEGORY_EFFECT: return "EffectError";
        case CATEGORY_PATTERN: return "PatternError";
        case CATEGORY_MODULE: return "ModuleError";
        case CATEGORY_API: return "ApiError";
        case CATEGORY_RUNTIME: return "RuntimeError";
        case CATEGORY_RESOURCE: return "ResourceError";
        case CATEGORY_INTERNAL: return "InternalError";
        default: return "UnknownError";
    }
}

const char* get_error_message(uint32_t code) {
    switch (code) {
        // Syntax errors
        case E1001_UNEXPECTED_TOKEN: return "Unexpected token";
        case E1002_MISSING_CLOSING_BRACE: return "Missing '}' to close block";
        case E1003_INVALID_FUNCTION_DECLARATION: return "Invalid function declaration syntax";
        case E1004_INVALID_API_DECLARATION: return "Invalid API declaration";
        case E1005_INVALID_TYPE_DECLARATION: return "Invalid type declaration";
        case E1006_EMPTY_BLOCK_NOT_ALLOWED: return "Empty blocks are not allowed";

        // Type errors
        case E2001_UNKNOWN_IDENTIFIER: return "Unknown identifier";
        case E2002_TYPE_MISMATCH: return "Type mismatch";
        case E2003_INVALID_FUNCTION_CALL: return "Invalid function call";
        case E2004_INVALID_RETURN_TYPE: return "Invalid return type";
        case E2005_MISSING_RETURN_VALUE: return "Function does not return a value on all paths";
        case E2006_REASSIGNMENT_FORBIDDEN: return "Reassignment of immutable variable is not allowed";
        case E2007_INVALID_CONDITION_TYPE: return "Condition expression must be Bool";

        // Effect errors
        case E3001_UNDECLARED_EFFECT_USAGE: return "Effect is used but not declared";
        case E3002_EFFECT_LEAKAGE: return "Pure function cannot call effectful function";
        case E3003_EFFECT_ESCALATION: return "Function declares effect but uses undeclared effect";
        case E3004_EFFECT_USAGE_IN_LAMBDA: return "Lambdas must be pure and cannot use effects";
        case E3005_INVALID_EFFECT_DECLARATION: return "Effect is not defined";

        // Pattern errors
        case E4001_NON_EXHAUSTIVE_MATCH: return "Pattern match is not exhaustive";
        case E4002_INVALID_MATCH_TARGET: return "Match expression must be an algebraic data type";
        case E4003_DUPLICATE_PATTERN: return "Duplicate pattern in match expression";
        case E4004_INCONSISTENT_MATCH_RESULT_TYPES: return "All match branches must return the same type";

        // Module errors
        case E5001_MODULE_NOT_FOUND: return "Module not found";
        case E5002_DUPLICATE_MODULE_DEFINITION: return "Module is defined more than once";
        case E5003_SYMBOL_NOT_EXPORTED: return "Symbol is not exported by module";
        case E5004_CIRCULAR_DEPENDENCY: return "Circular dependency detected between modules";

        // API errors
        case E6001_INVALID_HTTP_METHOD: return "Invalid HTTP method";
        case E6002_INVALID_ROUTE_PATH: return "Invalid route path";
        case E6003_MISSING_API_RESPONSE: return "API handler must return a Response";
        case E6004_INVALID_API_PARAMETER_TYPE: return "API parameter must be Int";
        case E6005_UNDECLARED_EFFECT_IN_API: return "API uses effect but it is not declared";

        // Runtime errors
        case E7001_INVALID_RESULT_VALUE: return "Invalid Result value returned";
        case E7002_INVALID_OPTION_VALUE: return "Invalid Option value returned";
        case E7003_SERIALIZATION_FAILURE: return "Failed to serialize response body";
        case E7004_INVALID_BYTECODE: return "Invalid or corrupted bytecode";

        // Resource errors
        case E8001_EXECUTION_TIMEOUT: return "Execution time limit exceeded";
        case E8002_MEMORY_LIMIT_EXCEEDED: return "Memory limit exceeded";
        case E8003_RECURSION_LIMIT_EXCEEDED: return "Maximum recursion depth exceeded";
        case E8004_ALLOCATION_LIMIT_EXCEEDED: return "Too many allocations";

        // Internal errors
        case E9001_COMPILER_INTERNAL_ERROR: return "Internal compiler error";
        case E9002_VM_INTERNAL_ERROR: return "Internal runtime error";

        default: return "Unknown error";
    }
}

// Error reporting functions
void report_error(CompilerError* error) {
    if (!error) return;

    fprintf(stderr, "Error E%04u (%s): %s",
            error->code,
            get_error_category_name(error->category),
            error->message);

    if (error->file) {
        fprintf(stderr, " at %s:%u:%u", error->file, error->line, error->column);
    }

    fprintf(stderr, "\n");
}

void report_syntax_error(SyntaxError code, const char* message,
                        const char* file, uint32_t line, uint32_t column) {
    CompilerError* error = create_error(code, message, file, line, column);
    report_error(error);
    free_error(error);
}

void report_type_error(TypeError code, const char* message,
                      const char* file, uint32_t line, uint32_t column) {
    CompilerError* error = create_error(code, message, file, line, column);
    report_error(error);
    free_error(error);
}

void report_effect_error(EffectError code, const char* message,
                        const char* file, uint32_t line, uint32_t column) {
    CompilerError* error = create_error(code, message, file, line, column);
    report_error(error);
    free_error(error);
}

void report_pattern_error(PatternError code, const char* message,
                         const char* file, uint32_t line, uint32_t column) {
    CompilerError* error = create_error(code, message, file, line, column);
    report_error(error);
    free_error(error);
}

void report_module_error(ModuleError code, const char* message,
                        const char* file, uint32_t line, uint32_t column) {
    CompilerError* error = create_error(code, message, file, line, column);
    report_error(error);
    free_error(error);
}

void report_api_error(ApiError code, const char* message,
                     const char* file, uint32_t line, uint32_t column) {
    CompilerError* error = create_error(code, message, file, line, column);
    report_error(error);
    free_error(error);
}

void report_runtime_error(RuntimeError code, const char* message,
                         const char* file, uint32_t line, uint32_t column) {
    CompilerError* error = create_error(code, message, file, line, column);
    report_error(error);
    free_error(error);
}

void report_resource_error(ResourceError code, const char* message,
                          const char* file, uint32_t line, uint32_t column) {
    CompilerError* error = create_error(code, message, file, line, column);
    report_error(error);
    free_error(error);
}

void report_internal_error(InternalError code, const char* message,
                          const char* file, uint32_t line, uint32_t column) {
    CompilerError* error = create_error(code, message, file, line, column);
    report_error(error);
    free_error(error);
}
