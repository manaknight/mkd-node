#ifndef MANAKNIGHT_LEXER_H
#define MANAKNIGHT_LEXER_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

// Token types
typedef enum {
    TOK_EOF = 0,
    TOK_IDENTIFIER,
    TOK_INT_LITERAL,
    TOK_STRING_LITERAL,
    TOK_BOOL_LITERAL,
    TOK_UNIT_LITERAL,

    // Keywords
    TOK_FN,
    TOK_LET,
    TOK_IF,
    TOK_ELSE,
    TOK_MATCH,
    TOK_TYPE,
    TOK_EFFECT,
    TOK_IMPORT,
    TOK_API,
    TOK_GET,
    TOK_POST,
    TOK_PUT,
    TOK_DELETE,
    TOK_HEAD,

    // Symbols
    TOK_LPAREN,
    TOK_RPAREN,
    TOK_LBRACE,
    TOK_RBRACE,
    TOK_LBRACKET,
    TOK_RBRACKET,
    TOK_COMMA,
    TOK_COLON,
    TOK_SEMICOLON,
    TOK_DOT,
    TOK_PIPE,
    TOK_ARROW,
    TOK_EQUALS,
    TOK_DOUBLE_EQUALS,
    TOK_NOT_EQUALS,
    TOK_LESS,
    TOK_GREATER,
    TOK_LESS_EQUALS,
    TOK_GREATER_EQUALS,
    TOK_PLUS,
    TOK_MINUS,
    TOK_STAR,
    TOK_SLASH,
    TOK_PERCENT,
    TOK_EXCLAMATION,
    TOK_QUESTION,
    TOK_AMPERSAND,
    TOK_DOUBLE_AMPERSAND,
    TOK_DOUBLE_PIPE,
    TOK_UNDERSCORE,

    TOK_INVALID
} TokenType;

// Token structure
typedef struct {
    TokenType type;
    char* text;
    uint32_t line;
    uint32_t column;
    union {
        int64_t int_val;
        char* string_val;
        bool bool_val;
    } value;
} Token;

// Lexer structure
typedef struct {
    const char* source;
    size_t source_len;
    size_t position;
    uint32_t line;
    uint32_t column;
    Token current_token;
} Lexer;

// Lexer functions
Lexer* lexer_create(const char* source, const char* filename);
void lexer_free(Lexer* lexer);
Token lexer_next_token(Lexer* lexer);
Token lexer_peek_token(Lexer* lexer);
const char* token_type_to_string(TokenType type);

#endif // MANAKNIGHT_LEXER_H