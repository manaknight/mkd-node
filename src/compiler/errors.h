#ifndef MANAKNIGHT_ERRORS_H
#define MANAKNIGHT_ERRORS_H

#include <stdint.h>

// Error codes (E1xxx-E9xxx)
typedef enum {
    // Syntax errors (E1000-E1999)
    ERR_SYNTAX_UNEXPECTED_TOKEN = 1000,
    ERR_SYNTAX_MISSING_TOKEN = 1001,
    ERR_SYNTAX_INVALID_LITERAL = 1002,

    // Type errors (E2000-E2999)
    ERR_TYPE_UNDEFINED_SYMBOL = 2000,
    ERR_TYPE_TYPE_MISMATCH = 2001,
    ERR_TYPE_INVALID_OPERATION = 2002,
    ERR_TYPE_SHADOWING = 2006,

    // Effect errors (E3000-E3999)
    ERR_EFFECT_PURE_FUNCTION_CALL = 3002,
    ERR_EFFECT_LAMBDA_NOT_PURE = 3004,

    // Pattern matching errors (E4000-E4999)
    ERR_PATTERN_NON_EXHAUSTIVE = 4001,
    ERR_PATTERN_DUPLICATE_CONSTRUCTOR = 4003,

    // Module errors (E5000-E5999)
    ERR_MODULE_CIRCULAR_DEPENDENCY = 5004,

    // API errors (E6000-E6999)
    // Runtime errors (E7000-E7999)
    // Resource limit errors (E8000-E8999)
    // Internal errors (E9000-E9999)
} ErrorCode;

// Error structure
typedef struct {
    ErrorCode code;
    char* message;
    char* filename;
    uint32_t line;
    uint32_t column;
} CompilerError;

// Error reporting functions
void error_report(ErrorCode code, const char* message, const char* filename,
                  uint32_t line, uint32_t column);
CompilerError* error_create(ErrorCode code, const char* message,
                           const char* filename, uint32_t line, uint32_t column);
void error_free(CompilerError* error);

// Error message formatting
const char* error_code_to_string(ErrorCode code);

#endif // MANAKNIGHT_ERRORS_H