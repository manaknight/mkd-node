#ifndef PARSER_H
#define PARSER_H

#include "lexer.h"
#include "ast.h"
#include "errors.h"

// Parser structure
typedef struct {
    Lexer* lexer;
    Token current_token;
    Token previous_token;
} Parser;

// Function declarations
Parser* create_parser(Lexer* lexer);
void free_parser(Parser* parser);

// Main parsing function
Program* parse_program(Parser* parser);

// Helper functions
void advance(Parser* parser);
bool check(Parser* parser, TokenType type);
bool match(Parser* parser, TokenType type);
Token consume(Parser* parser, TokenType type, const char* error_message);

// Parsing functions for different AST nodes
Module* parse_module(Parser* parser);
ApiRoute* parse_api(Parser* parser);
Decl* parse_declaration(Parser* parser);
Stmt* parse_statement(Parser* parser);
Expr* parse_expression(Parser* parser);
Type* parse_type(Parser* parser);
Pattern* parse_pattern(Parser* parser);

// Specific parsing functions
char* parse_http_method(Parser* parser);
char* parse_path(Parser* parser);
Param* parse_parameters(Parser* parser, size_t* param_count);
Type** parse_effects(Parser* parser, size_t* effect_count);
char** parse_effect_list(Parser* parser, size_t* effect_count);
MatchCase* parse_match_cases(Parser* parser, size_t* case_count);

#endif // PARSER_H
