#ifndef MANAKNIGHT_PARSER_H
#define MANAKNIGHT_PARSER_H

#include "ast.h"
#include "lexer.h"

// Parser structure
typedef struct {
    Lexer* lexer;
    Token current_token;
    char* filename;
} Parser;

// Parser functions
Parser* parser_create(Lexer* lexer, const char* filename);
void parser_free(Parser* parser);
Program* parser_parse_program(Parser* parser);

// Internal parsing functions
FunctionDecl* parser_parse_function(Parser* parser);
Block* parser_parse_block(Parser* parser);
void parser_skip_to_next_declaration(Parser* parser);

#endif // MANAKNIGHT_PARSER_H