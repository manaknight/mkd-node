#include "lexer.h"
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>

// Lexer functions
Lexer* lexer_create(const char* source, const char* filename) {
    Lexer* lexer = calloc(1, sizeof(Lexer));
    if (!lexer) return NULL;

    lexer->source = source;
    lexer->source_len = strlen(source);
    lexer->position = 0;
    lexer->line = 1;
    lexer->column = 1;

    return lexer;
}

void lexer_free(Lexer* lexer) {
    if (!lexer) return;
    if (lexer->current_token.text) free(lexer->current_token.text);
    free(lexer);
}

Token lexer_next_token(Lexer* lexer) {
    // Skip whitespace and comments
    while (lexer->position < lexer->source_len) {
        char c = lexer->source[lexer->position];

        if (c == ' ' || c == '\t' || c == '\r') {
            lexer->position++;
            lexer->column++;
        } else if (c == '\n') {
            lexer->position++;
            lexer->line++;
            lexer->column = 1;
        } else if (c == '/' && lexer->position + 1 < lexer->source_len &&
                   lexer->source[lexer->position + 1] == '/') {
            // Skip comment
            while (lexer->position < lexer->source_len &&
                   lexer->source[lexer->position] != '\n') {
                lexer->position++;
            }
        } else {
            break;
        }
    }

    if (lexer->position >= lexer->source_len) {
        Token token = {
            .type = TOK_EOF,
            .text = NULL,
            .line = lexer->line,
            .column = lexer->column
        };
        return token;
    }


    char c = lexer->source[lexer->position];
    uint32_t start_line = lexer->line;
    uint32_t start_column = lexer->column;

    // Handle identifiers and keywords
    if (isalpha(c) || c == '_') {
        size_t start = lexer->position;
        while (lexer->position < lexer->source_len &&
               (isalnum(lexer->source[lexer->position]) || lexer->source[lexer->position] == '_')) {
            lexer->position++;
            lexer->column++;
        }

        size_t length = lexer->position - start;
        char* text = strndup(lexer->source + start, length);

        TokenType type = TOK_IDENTIFIER;

        // Check for keywords
        if (strcmp(text, "fn") == 0) type = TOK_FN;
        else if (strcmp(text, "let") == 0) type = TOK_LET;
        else if (strcmp(text, "if") == 0) type = TOK_IF;
        else if (strcmp(text, "else") == 0) type = TOK_ELSE;
        else if (strcmp(text, "match") == 0) type = TOK_MATCH;
        else if (strcmp(text, "type") == 0) type = TOK_TYPE;
        else if (strcmp(text, "effect") == 0) type = TOK_EFFECT;
        else if (strcmp(text, "import") == 0) type = TOK_IMPORT;
        else if (strcmp(text, "api") == 0) type = TOK_API;
        else if (strcmp(text, "get") == 0) type = TOK_GET;
        else if (strcmp(text, "true") == 0) type = TOK_BOOL_LITERAL;
        else if (strcmp(text, "false") == 0) type = TOK_BOOL_LITERAL;
        else if (strcmp(text, "unit") == 0) type = TOK_UNIT_LITERAL;

        Token token = {
            .type = type,
            .text = text,
            .line = start_line,
            .column = start_column
        };

        if (type == TOK_BOOL_LITERAL) {
            token.value.bool_val = (strcmp(text, "true") == 0);
        }

        return token;
    }

    // Handle string literals
    if (c == '"') {
        lexer->position++; // skip opening quote
        lexer->column++;
        size_t start = lexer->position;

        while (lexer->position < lexer->source_len && lexer->source[lexer->position] != '"') {
            if (lexer->source[lexer->position] == '\\') {
                lexer->position++; // skip escape
                lexer->column++;
            }
            lexer->position++;
            lexer->column++;
        }

        if (lexer->position < lexer->source_len) {
            lexer->position++; // skip closing quote
            lexer->column++;
        }

        size_t length = lexer->position - start - 1; // -1 for closing quote
        char* text = strndup(lexer->source + start, length);

        Token token = {
            .type = TOK_STRING_LITERAL,
            .text = text,
            .line = start_line,
            .column = start_column,
            .value.string_val = text
        };
        return token;
    }

    // Handle numbers
    if (isdigit(c)) {
        size_t start = lexer->position;
        while (lexer->position < lexer->source_len && isdigit(lexer->source[lexer->position])) {
            lexer->position++;
            lexer->column++;
        }

        size_t length = lexer->position - start;
        char* text = strndup(lexer->source + start, length);

        Token token = {
            .type = TOK_INT_LITERAL,
            .text = text,
            .line = start_line,
            .column = start_column,
            .value.int_val = atoll(text)
        };
        return token;
    }

    // Handle symbols
    lexer->position++;
    lexer->column++;

    TokenType type = TOK_INVALID;
    char* text = NULL;

    switch (c) {
        case '(': type = TOK_LPAREN; text = strdup("("); break;
        case ')': type = TOK_RPAREN; text = strdup(")"); break;
        case '{': type = TOK_LBRACE; text = strdup("{"); break;
        case '}': type = TOK_RBRACE; text = strdup("}"); break;
        case '[': type = TOK_LBRACKET; text = strdup("["); break;
        case ']': type = TOK_RBRACKET; text = strdup("]"); break;
        case ',': type = TOK_COMMA; text = strdup(","); break;
        case ':': type = TOK_COLON; text = strdup(":"); break;
        case ';': type = TOK_SEMICOLON; text = strdup(";"); break;
        case '.': type = TOK_DOT; text = strdup("."); break;
        case '|': type = TOK_PIPE; text = strdup("|"); break;
        case '=': type = TOK_EQUALS; text = strdup("="); break;
        case '+': type = TOK_PLUS; text = strdup("+"); break;
        case '-':
            if (lexer->position < lexer->source_len && lexer->source[lexer->position] == '>') {
                lexer->position++;
                lexer->column++;
                type = TOK_ARROW;
                text = strdup("->");
            } else {
                type = TOK_MINUS;
                text = strdup("-");
            }
            break;
        case '*': type = TOK_STAR; text = strdup("*"); break;
        case '/': type = TOK_SLASH; text = strdup("/"); break;
        case '%': type = TOK_PERCENT; text = strdup("%"); break;
        case '!': type = TOK_EXCLAMATION; text = strdup("!"); break;
        case '?': type = TOK_QUESTION; text = strdup("?"); break;
        case '&': type = TOK_AMPERSAND; text = strdup("&"); break;
        case '_': type = TOK_UNDERSCORE; text = strdup("_"); break;
        default:
            type = TOK_INVALID;
            text = strndup(&c, 1);
            break;
    }

    Token token = {
        .type = type,
        .text = text,
        .line = start_line,
        .column = start_column
    };
    return token;
}

Token lexer_peek_token(Lexer* lexer) {
    // TODO: Implement peek
    return lexer_next_token(lexer);
}

const char* token_type_to_string(TokenType type) {
    switch (type) {
        case TOK_EOF: return "EOF";
        case TOK_IDENTIFIER: return "IDENTIFIER";
        case TOK_FN: return "fn";
        case TOK_LET: return "let";
        // TODO: Add more cases
        default: return "UNKNOWN";
    }
}