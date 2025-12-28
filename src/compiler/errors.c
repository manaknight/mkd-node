#include "errors.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// Error reporting functions
void error_report(ErrorCode code, const char* message, const char* filename,
                  uint32_t line, uint32_t column) {
    fprintf(stderr, "[E%04d] %s:%u:%u: %s\n",
            code, filename, line, column, message);
}

CompilerError* error_create(ErrorCode code, const char* message,
                           const char* filename, uint32_t line, uint32_t column) {
    CompilerError* error = malloc(sizeof(CompilerError));
    if (!error) return NULL;

    error->code = code;
    error->message = strdup(message);
    error->filename = strdup(filename);
    error->line = line;
    error->column = column;

    return error;
}

void error_free(CompilerError* error) {
    if (!error) return;
    free(error->message);
    free(error->filename);
    free(error);
}

// Error message formatting
const char* error_code_to_string(ErrorCode code) {
    switch (code) {
        case ERR_SYNTAX_UNEXPECTED_TOKEN: return "E1000";
        case ERR_SYNTAX_MISSING_TOKEN: return "E1001";
        case ERR_SYNTAX_INVALID_LITERAL: return "E1002";
        case ERR_TYPE_UNDEFINED_SYMBOL: return "E2000";
        case ERR_TYPE_TYPE_MISMATCH: return "E2001";
        case ERR_TYPE_INVALID_OPERATION: return "E2002";
        case ERR_TYPE_SHADOWING: return "E2006";
        case ERR_EFFECT_PURE_FUNCTION_CALL: return "E3002";
        case ERR_EFFECT_LAMBDA_NOT_PURE: return "E3004";
        case ERR_PATTERN_NON_EXHAUSTIVE: return "E4001";
        case ERR_PATTERN_DUPLICATE_CONSTRUCTOR: return "E4003";
        case ERR_MODULE_CIRCULAR_DEPENDENCY: return "E5004";
        default: return "E0000";
    }
}