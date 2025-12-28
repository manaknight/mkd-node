#include "lexer.h"
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>

// Lexer creation and destruction
Lexer* create_lexer(const char* source, uint32_t length) {
    Lexer* lexer = malloc(sizeof(Lexer));
    lexer->source = source;
    lexer->source_length = length;
    lexer->current = 0;
    lexer->start = 0;
    lexer->line = 1;
    lexer->column = 1;
    return lexer;
}

void free_lexer(Lexer* lexer) {
    free(lexer);
}

// Helper functions
static bool is_at_end(Lexer* lexer) {
    return lexer->current >= lexer->source_length;
}

static char peek(Lexer* lexer) {
    if (is_at_end(lexer)) return '\0';
    return lexer->source[lexer->current];
}

static char peek_next(Lexer* lexer) {
    if (lexer->current + 1 >= lexer->source_length) return '\0';
    return lexer->source[lexer->current + 1];
}

static char advance(Lexer* lexer) {
    char c = lexer->source[lexer->current++];
    if (c == '\n') {
        lexer->line++;
        lexer->column = 1;
    } else {
        lexer->column++;
    }
    return c;
}

static bool match(Lexer* lexer, char expected) {
    if (is_at_end(lexer)) return false;
    if (lexer->source[lexer->current] != expected) return false;
    advance(lexer);
    return true;
}

static Token make_token(Lexer* lexer, TokenType type) {
    Token token;
    token.type = type;
    token.lexeme = &lexer->source[lexer->start];
    token.length = lexer->current - lexer->start;
    token.line = lexer->line;
    token.column = lexer->column - token.length; // Column at start of token
    return token;
}

static Token make_literal_token(Lexer* lexer, TokenType type) {
    Token token = make_token(lexer, type);
    if (type == TOKEN_INT_LITERAL) {
        // Parse integer literal
        char* end;
        token.literal.int_val = strtoll(token.lexeme, &end, 10);
    } else if (type == TOKEN_STRING_LITERAL) {
        // String literal (without quotes)
        token.literal.string_val = strndup(token.lexeme + 1, token.length - 2);
    }
    return token;
}

static Token make_error_token(Lexer* lexer, const char* message) {
    Token token;
    token.type = TOKEN_ERROR;
    token.lexeme = message;
    token.length = strlen(message);
    token.line = lexer->line;
    token.column = lexer->column;
    return token;
}

// Skip whitespace and comments
static void skip_whitespace_and_comments(Lexer* lexer) {
    for (;;) {
        char c = peek(lexer);
        switch (c) {
            case ' ':
            case '\r':
            case '\t':
                advance(lexer);
                break;
            case '\n':
                advance(lexer);
                break;
            case '/':
                if (peek_next(lexer) == '/') {
                    // Skip single-line comment
                    while (peek(lexer) != '\n' && !is_at_end(lexer)) {
                        advance(lexer);
                    }
                } else {
                    return;
                }
                break;
            default:
                return;
        }
    }
}

// Check if character is valid identifier start
static bool is_identifier_start(char c) {
    return isalpha(c) || c == '_';
}

// Check if character is valid identifier continuation
static bool is_identifier_char(char c) {
    return isalnum(c) || c == '_';
}

// Parse identifier or keyword
static Token identifier_or_keyword(Lexer* lexer) {
    while (is_identifier_char(peek(lexer))) {
        advance(lexer);
    }

    // Check for keywords
    uint32_t length = lexer->current - lexer->start;
    const char* text = &lexer->source[lexer->start];

#define CHECK_KEYWORD(keyword, token_type) \
    if (length == strlen(#keyword) && memcmp(text, #keyword, length) == 0) { \
        return make_token(lexer, token_type); \
    }

    CHECK_KEYWORD(function, TOKEN_FUNCTION);
    CHECK_KEYWORD(let, TOKEN_LET);
    CHECK_KEYWORD(if, TOKEN_IF);
    CHECK_KEYWORD(else, TOKEN_ELSE);
    CHECK_KEYWORD(match, TOKEN_MATCH);
    CHECK_KEYWORD(effect, TOKEN_EFFECT);
    CHECK_KEYWORD(api, TOKEN_API);
    CHECK_KEYWORD(module, TOKEN_MODULE);
    CHECK_KEYWORD(type, TOKEN_TYPE);
    CHECK_KEYWORD(uses, TOKEN_USES);
    CHECK_KEYWORD(fn, TOKEN_FN);
    CHECK_KEYWORD(true, TOKEN_TRUE);
    CHECK_KEYWORD(false, TOKEN_FALSE);
    CHECK_KEYWORD(some, TOKEN_SOME);
    CHECK_KEYWORD(none, TOKEN_NONE);
    CHECK_KEYWORD(ok, TOKEN_OK);
    CHECK_KEYWORD(err, TOKEN_ERR);

#undef CHECK_KEYWORD

    return make_token(lexer, TOKEN_IDENTIFIER);
}

// Parse number
static Token number(Lexer* lexer) {
    while (isdigit(peek(lexer))) {
        advance(lexer);
    }

    return make_literal_token(lexer, TOKEN_INT_LITERAL);
}

// Parse string
static Token string(Lexer* lexer) {
    while (peek(lexer) != '"' && !is_at_end(lexer)) {
        if (peek(lexer) == '\n') {
            return make_error_token(lexer, "Unterminated string literal");
        }
        advance(lexer);
    }

    if (is_at_end(lexer)) {
        return make_error_token(lexer, "Unterminated string literal");
    }

    // Consume the closing quote
    advance(lexer);

    return make_literal_token(lexer, TOKEN_STRING_LITERAL);
}

// Main token scanning function
Token next_token(Lexer* lexer) {
    skip_whitespace_and_comments(lexer);

    lexer->start = lexer->current;

    if (is_at_end(lexer)) {
        return make_token(lexer, TOKEN_EOF);
    }

    char c = advance(lexer);

    // Single-character tokens
    switch (c) {
        case '(': return make_token(lexer, TOKEN_LPAREN);
        case ')': return make_token(lexer, TOKEN_RPAREN);
        case '{': return make_token(lexer, TOKEN_LBRACE);
        case '}': return make_token(lexer, TOKEN_RBRACE);
        case '[': return make_token(lexer, TOKEN_LBRACKET);
        case ']': return make_token(lexer, TOKEN_RBRACKET);
        case ',': return make_token(lexer, TOKEN_COMMA);
        case ':': return make_token(lexer, TOKEN_COLON);
        case ';': return make_token(lexer, TOKEN_SEMICOLON);
        case '.': return make_token(lexer, TOKEN_DOT);
        case '|': return make_token(lexer, TOKEN_PIPE);
        case '+': return make_token(lexer, TOKEN_PLUS);
        case '-': return make_token(lexer, TOKEN_MINUS);
        case '*': return make_token(lexer, TOKEN_STAR);
        case '/': return make_token(lexer, TOKEN_SLASH);
        case '%': return make_token(lexer, TOKEN_MOD);
        case '=': return make_token(lexer, TOKEN_EQUALS_SIGN);
        case '<': return make_token(lexer, TOKEN_LESS);
        case '>': return make_token(lexer, TOKEN_GREATER);
        case '!': return make_token(lexer, TOKEN_NOT);
        case '&': return make_token(lexer, TOKEN_AND);
        case 'o': return make_token(lexer, TOKEN_OR);
        case '"': return string(lexer);
    }

    // Two-character tokens
    if (c == '=' && match(lexer, '=')) return make_token(lexer, TOKEN_DOUBLE_EQUALS);
    if (c == '!' && match(lexer, '=')) return make_token(lexer, TOKEN_NOT_EQUALS);
    if (c == '<' && match(lexer, '=')) return make_token(lexer, TOKEN_LESS_EQUALS);
    if (c == '>' && match(lexer, '=')) return make_token(lexer, TOKEN_GREATER_EQUALS);
    if (c == '-' && match(lexer, '>')) return make_token(lexer, TOKEN_ARROW);
    if (c == '|' && match(lexer, '>')) return make_token(lexer, TOKEN_PIPE_RIGHT);

    // Numbers
    if (isdigit(c)) {
        return number(lexer);
    }

    // Identifiers/Keywords
    if (is_identifier_start(c)) {
        return identifier_or_keyword(lexer);
    }

    return make_error_token(lexer, "Unexpected character");
}

// Utility functions
const char* token_type_to_string(TokenType type) {
    switch (type) {
        case TOKEN_FUNCTION: return "FUNCTION";
        case TOKEN_LET: return "LET";
        case TOKEN_IF: return "IF";
        case TOKEN_ELSE: return "ELSE";
        case TOKEN_MATCH: return "MATCH";
        case TOKEN_EFFECT: return "EFFECT";
        case TOKEN_API: return "API";
        case TOKEN_MODULE: return "MODULE";
        case TOKEN_TYPE: return "TYPE";
        case TOKEN_USES: return "USES";
        case TOKEN_FN: return "FN";
        case TOKEN_TRUE: return "TRUE";
        case TOKEN_FALSE: return "FALSE";
        case TOKEN_SOME: return "SOME";
        case TOKEN_NONE: return "NONE";
        case TOKEN_OK: return "OK";
        case TOKEN_ERR: return "ERR";
        case TOKEN_INT_LITERAL: return "INT_LITERAL";
        case TOKEN_STRING_LITERAL: return "STRING_LITERAL";
        case TOKEN_IDENTIFIER: return "IDENTIFIER";
        case TOKEN_PLUS: return "PLUS";
        case TOKEN_MINUS: return "MINUS";
        case TOKEN_STAR: return "STAR";
        case TOKEN_SLASH: return "SLASH";
        case TOKEN_MOD: return "MOD";
        case TOKEN_EQUALS: return "EQUALS";
        case TOKEN_DOUBLE_EQUALS: return "DOUBLE_EQUALS";
        case TOKEN_NOT_EQUALS: return "NOT_EQUALS";
        case TOKEN_LESS: return "LESS";
        case TOKEN_LESS_EQUALS: return "LESS_EQUALS";
        case TOKEN_GREATER: return "GREATER";
        case TOKEN_GREATER_EQUALS: return "GREATER_EQUALS";
        case TOKEN_NOT: return "NOT";
        case TOKEN_AND: return "AND";
        case TOKEN_OR: return "OR";
        case TOKEN_LPAREN: return "LPAREN";
        case TOKEN_RPAREN: return "RPAREN";
        case TOKEN_LBRACE: return "LBRACE";
        case TOKEN_RBRACE: return "RBRACE";
        case TOKEN_LBRACKET: return "LBRACKET";
        case TOKEN_RBRACKET: return "RBRACKET";
        case TOKEN_COMMA: return "COMMA";
        case TOKEN_COLON: return "COLON";
        case TOKEN_SEMICOLON: return "SEMICOLON";
        case TOKEN_DOT: return "DOT";
        case TOKEN_PIPE: return "PIPE";
        case TOKEN_PIPE_RIGHT: return "PIPE_RIGHT";
        case TOKEN_ARROW: return "ARROW";
        case TOKEN_EQUALS_SIGN: return "EQUALS_SIGN";
        case TOKEN_EOF: return "EOF";
        case TOKEN_ERROR: return "ERROR";
        default: return "UNKNOWN";
    }
}

void print_token(Token token) {
    printf("Token { type: %s, lexeme: '%.*s', line: %u, column: %u",
           token_type_to_string(token.type),
           token.length,
           token.lexeme,
           token.line,
           token.column);

    if (token.type == TOKEN_INT_LITERAL) {
        printf(", value: %lld", token.literal.int_val);
    } else if (token.type == TOKEN_STRING_LITERAL) {
        printf(", value: \"%s\"", token.literal.string_val);
    }

    printf(" }\n");
}
