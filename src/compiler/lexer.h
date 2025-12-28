#ifndef LEXER_H
#define LEXER_H

#include <stdint.h>
#include <stdbool.h>

// Token types
typedef enum {
    // Keywords
    TOKEN_FUNCTION,
    TOKEN_LET,
    TOKEN_IF,
    TOKEN_ELSE,
    TOKEN_MATCH,
    TOKEN_EFFECT,
    TOKEN_API,
    TOKEN_MODULE,
    TOKEN_TYPE,
    TOKEN_USES,
    TOKEN_FN,
    TOKEN_TRUE,
    TOKEN_FALSE,
    TOKEN_SOME,
    TOKEN_NONE,
    TOKEN_OK,
    TOKEN_ERR,

    // Literals
    TOKEN_INT_LITERAL,
    TOKEN_STRING_LITERAL,
    TOKEN_IDENTIFIER,

    // Operators
    TOKEN_PLUS,
    TOKEN_MINUS,
    TOKEN_STAR,
    TOKEN_SLASH,
    TOKEN_MOD,
    TOKEN_EQUALS,
    TOKEN_DOUBLE_EQUALS,
    TOKEN_NOT_EQUALS,
    TOKEN_LESS,
    TOKEN_LESS_EQUALS,
    TOKEN_GREATER,
    TOKEN_GREATER_EQUALS,
    TOKEN_NOT,
    TOKEN_AND,
    TOKEN_OR,

    // Symbols
    TOKEN_LPAREN,
    TOKEN_RPAREN,
    TOKEN_LBRACE,
    TOKEN_RBRACE,
    TOKEN_LBRACKET,
    TOKEN_RBRACKET,
    TOKEN_COMMA,
    TOKEN_COLON,
    TOKEN_SEMICOLON,
    TOKEN_DOT,
    TOKEN_PIPE,
    TOKEN_PIPE_RIGHT,  // |>
    TOKEN_ARROW,       // ->
    TOKEN_EQUALS_SIGN, // =

    // Special tokens
    TOKEN_EOF,
    TOKEN_ERROR,
} TokenType;

// Token structure
typedef struct {
    TokenType type;
    const char* lexeme;    // The actual text
    uint32_t length;       // Length of lexeme
    uint32_t line;         // Line number (1-based)
    uint32_t column;       // Column number (1-based)
    union {
        int64_t int_val;
        char* string_val;
    } literal;
} Token;

// Lexer structure
typedef struct {
    const char* source;
    uint32_t source_length;
    uint32_t current;      // Current position in source
    uint32_t start;        // Start of current token
    uint32_t line;         // Current line
    uint32_t column;       // Current column
} Lexer;

// Function declarations
Lexer* create_lexer(const char* source, uint32_t length);
void free_lexer(Lexer* lexer);
Token next_token(Lexer* lexer);
const char* token_type_to_string(TokenType type);
void print_token(Token token);

#endif // LEXER_H
