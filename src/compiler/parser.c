#include "parser.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// Parser creation and destruction
Parser* create_parser(Lexer* lexer) {
    Parser* parser = malloc(sizeof(Parser));
    parser->lexer = lexer;
    parser->current_token = next_token(lexer);
    parser->previous_token = (Token){0}; // Initialize to zero
    return parser;
}

void free_parser(Parser* parser) {
    free(parser);
}

// Helper functions
void advance(Parser* parser) {
    parser->previous_token = parser->current_token;
    parser->current_token = next_token(parser->lexer);
}

bool check(Parser* parser, TokenType type) {
    return parser->current_token.type == type;
}

bool match(Parser* parser, TokenType type) {
    if (check(parser, type)) {
        advance(parser);
        return true;
    }
    return false;
}

Token consume(Parser* parser, TokenType type, const char* error_message) {
    if (check(parser, type)) {
        Token token = parser->current_token;
        advance(parser);
        return token;
    }

    // Report error and return error token
    report_syntax_error(E1001_UNEXPECTED_TOKEN, error_message,
                       NULL, parser->current_token.line, parser->current_token.column);
    return (Token){ .type = TOKEN_ERROR };
}

// Main parsing function
Program* parse_program(Parser* parser) {
    Module** modules = NULL;
    size_t module_count = 0;
    ApiRoute** apis = NULL;
    size_t api_count = 0;

    // Parse modules and APIs
    while (!check(parser, TOKEN_EOF)) {
        if (match(parser, TOKEN_MODULE)) {
            Module* module = parse_module(parser);
            if (module) {
                modules = realloc(modules, sizeof(Module*) * (module_count + 1));
                modules[module_count++] = module;
            }
        } else if (match(parser, TOKEN_API)) {
            ApiRoute* api = parse_api(parser);
            if (api) {
                apis = realloc(apis, sizeof(ApiRoute*) * (api_count + 1));
                apis[api_count++] = api;
            }
        } else {
            // Skip unexpected tokens or report error
            report_syntax_error(E1001_UNEXPECTED_TOKEN, "Expected module or api declaration",
                               NULL, parser->current_token.line, parser->current_token.column);
            advance(parser);
        }
    }

    return create_program(modules, module_count, apis, api_count);
}

// Parse module declaration
Module* parse_module(Parser* parser) {
    // Module name
    Token name_token = consume(parser, TOKEN_IDENTIFIER, "Expected module name");
    if (name_token.type == TOKEN_ERROR) return NULL;

    consume(parser, TOKEN_LBRACE, "Expected '{' after module name");

    // Parse declarations
    Decl** decls = NULL;
    size_t decl_count = 0;

    while (!check(parser, TOKEN_RBRACE) && !check(parser, TOKEN_EOF)) {
        Decl* decl = parse_declaration(parser);
        if (decl) {
            decls = realloc(decls, sizeof(Decl*) * (decl_count + 1));
            decls[decl_count++] = decl;
        }
    }

    consume(parser, TOKEN_RBRACE, "Expected '}' after module declarations");

    char* name = strndup(name_token.lexeme, name_token.length);
    return create_module(name, decls, decl_count);
}

// Parse API declaration
ApiRoute* parse_api(Parser* parser) {
    // HTTP method
    char* method = parse_http_method(parser);
    if (!method) return NULL;

    // Path
    char* path = parse_path(parser);
    if (!path) return NULL;

    consume(parser, TOKEN_LBRACE, "Expected '{' after API declaration");

    // Parse body (statements)
    Stmt* body = parse_statement(parser); // Simplified - should parse block

    consume(parser, TOKEN_RBRACE, "Expected '}' after API body");

    return create_api_route(method, path, body);
}

// Parse HTTP method
char* parse_http_method(Parser* parser) {
    if (!check(parser, TOKEN_IDENTIFIER)) {
        report_syntax_error(E1001_UNEXPECTED_TOKEN, "Expected HTTP method",
                           NULL, parser->current_token.line, parser->current_token.column);
        return NULL;
    }

    Token method_token = parser->current_token;
    const char* methods[] = {"GET", "POST", "PUT", "DELETE", "PATCH", "HEAD", "OPTIONS"};
    size_t method_count = sizeof(methods) / sizeof(methods[0]);

    bool valid_method = false;
    for (size_t i = 0; i < method_count; i++) {
        if (method_token.length == strlen(methods[i]) &&
            memcmp(method_token.lexeme, methods[i], method_token.length) == 0) {
            valid_method = true;
            break;
        }
    }

    if (!valid_method) {
        report_syntax_error(E6001_INVALID_HTTP_METHOD, "Invalid HTTP method",
                           NULL, method_token.line, method_token.column);
        return NULL;
    }

    advance(parser);
    return strndup(method_token.lexeme, method_token.length);
}

// Parse API path
char* parse_path(Parser* parser) {
    if (!check(parser, TOKEN_STRING_LITERAL)) {
        report_syntax_error(E1001_UNEXPECTED_TOKEN, "Expected path string",
                           NULL, parser->current_token.line, parser->current_token.column);
        return NULL;
    }

    Token path_token = parser->current_token;
    advance(parser);
    return strndup(path_token.lexeme, path_token.length);
}

// Parse declaration
Decl* parse_declaration(Parser* parser) {
    if (match(parser, TOKEN_FUNCTION)) {
        return parse_function_declaration(parser);
    } else if (match(parser, TOKEN_TYPE)) {
        return parse_type_declaration(parser);
    } else if (match(parser, TOKEN_EFFECT)) {
        return parse_effect_declaration(parser);
    } else if (match(parser, TOKEN_IDENTIFIER)) {
        // Could be import - check for module path
        if (check(parser, TOKEN_IDENTIFIER) || check(parser, TOKEN_STRING_LITERAL)) {
            return parse_import_declaration(parser);
        }
    }

    report_syntax_error(E1001_UNEXPECTED_TOKEN, "Expected declaration",
                       NULL, parser->current_token.line, parser->current_token.column);
    return NULL;
}

// Parse function declaration
Decl* parse_function_declaration(Parser* parser) {
    // Function name
    Token name_token = consume(parser, TOKEN_IDENTIFIER, "Expected function name");
    if (name_token.type == TOKEN_ERROR) return NULL;

    consume(parser, TOKEN_LPAREN, "Expected '(' after function name");

    // Parameters
    size_t param_count;
    Param* params = parse_parameters(parser, &param_count);

    consume(parser, TOKEN_RPAREN, "Expected ')' after parameters");

    // Return type (optional)
    Type* return_type = create_primitive_type(PRIM_INT); // Default to Int
    if (match(parser, TOKEN_COLON)) {
        return_type = parse_type(parser);
    }

    // Effects (optional)
    char** effects = NULL;
    size_t effect_count = 0;
    if (match(parser, TOKEN_USES)) {
        consume(parser, TOKEN_LBRACE, "Expected '{' after 'uses'");
        effects = parse_effect_list(parser, &effect_count);
        consume(parser, TOKEN_RBRACE, "Expected '}' after effects");
    }

    // Body
    consume(parser, TOKEN_LBRACE, "Expected '{' before function body");

    // For now, parse a simple statement as body
    Stmt* body = parse_statement(parser);

    consume(parser, TOKEN_RBRACE, "Expected '}' after function body");

    char* name = strndup(name_token.lexeme, name_token.length);
    return create_function_decl(name, params, param_count, return_type, effects, effect_count, body);
}

// Parse parameters
Param* parse_parameters(Parser* parser, size_t* param_count) {
    Param* params = NULL;
    *param_count = 0;

    if (check(parser, TOKEN_RPAREN)) {
        return NULL; // No parameters
    }

    do {
        Token param_name = consume(parser, TOKEN_IDENTIFIER, "Expected parameter name");
        if (param_name.type == TOKEN_ERROR) return NULL;

        consume(parser, TOKEN_COLON, "Expected ':' after parameter name");

        Type* param_type = parse_type(parser);
        if (!param_type) return NULL;

        params = realloc(params, sizeof(Param) * (*param_count + 1));
        params[*param_count].name = strndup(param_name.lexeme, param_name.length);
        params[*param_count].type = param_type;
        (*param_count)++;
    } while (match(parser, TOKEN_COMMA));

    return params;
}

// Parse effects list
char** parse_effect_list(Parser* parser, size_t* effect_count) {
    char** effects = NULL;
    *effect_count = 0;

    do {
        Token effect_token = consume(parser, TOKEN_IDENTIFIER, "Expected effect name");
        if (effect_token.type == TOKEN_ERROR) return NULL;

        effects = realloc(effects, sizeof(char*) * (*effect_count + 1));
        effects[*effect_count] = strndup(effect_token.lexeme, effect_token.length);
        (*effect_count)++;
    } while (match(parser, TOKEN_COMMA));

    return effects;
}

// Parse type declaration
Decl* parse_type_declaration(Parser* parser) {
    Token name_token = consume(parser, TOKEN_IDENTIFIER, "Expected type name");
    if (name_token.type == TOKEN_ERROR) return NULL;

    // For now, create a simple type declaration
    // This would need to be expanded to handle record vs union types
    char* name = strndup(name_token.lexeme, name_token.length);
    return create_type_decl(name, false, NULL);
}

// Parse effect declaration
Decl* parse_effect_declaration(Parser* parser) {
    Token name_token = consume(parser, TOKEN_IDENTIFIER, "Expected effect name");
    if (name_token.type == TOKEN_ERROR) return NULL;

    char* name = strndup(name_token.lexeme, name_token.length);
    return create_effect_decl(name);
}

// Parse import declaration
Decl* parse_import_declaration(Parser* parser) {
    // This is a simplified version
    Token module_token = parser->previous_token; // The identifier we already consumed
    Token alias_token = {0};

    if (match(parser, TOKEN_IDENTIFIER)) {
        alias_token = parser->previous_token;
    }

    char* module_path = strndup(module_token.lexeme, module_token.length);
    char* alias = alias_token.lexeme ? strndup(alias_token.lexeme, alias_token.length) : NULL;

    return create_import_decl(module_path, alias);
}

// Parse statement
Stmt* parse_statement(Parser* parser) {
    if (match(parser, TOKEN_LET)) {
        return parse_let_statement(parser);
    } else if (match(parser, TOKEN_IF)) {
        return parse_if_statement(parser);
    } else if (match(parser, TOKEN_MATCH)) {
        return parse_match_statement(parser);
    } else if (match(parser, TOKEN_LBRACE)) {
        // Block statement
        Stmt* stmt = parse_statement(parser);
        consume(parser, TOKEN_RBRACE, "Expected '}' after block");
        return stmt;
    } else {
        // Expression statement
        Expr* expr = parse_expression(parser);
        return create_expr_stmt(expr);
    }
}

// Parse let statement
Stmt* parse_let_statement(Parser* parser) {
    Token name_token = consume(parser, TOKEN_IDENTIFIER, "Expected variable name");
    if (name_token.type == TOKEN_ERROR) return NULL;

    consume(parser, TOKEN_EQUALS_SIGN, "Expected '=' after variable name");

    Expr* expr = parse_expression(parser);
    if (!expr) return NULL;

    char* name = strndup(name_token.lexeme, name_token.length);
    return create_let_stmt(name, expr);
}

// Parse if statement
Stmt* parse_if_statement(Parser* parser) {
    Expr* condition = parse_expression(parser);
    if (!condition) return NULL;

    Stmt* then_branch = parse_statement(parser);
    if (!then_branch) return NULL;

    Stmt* else_branch = NULL;
    if (match(parser, TOKEN_ELSE)) {
        else_branch = parse_statement(parser);
    }

    return create_if_stmt(condition, then_branch, else_branch);
}

// Parse match statement
Stmt* parse_match_statement(Parser* parser) {
    Expr* scrutinee = parse_expression(parser);
    if (!scrutinee) return NULL;

    consume(parser, TOKEN_LBRACE, "Expected '{' after match expression");

    // Simplified - parse one case for now
    MatchCase* cases = NULL;
    size_t case_count = 0;

    while (!check(parser, TOKEN_RBRACE) && !check(parser, TOKEN_EOF)) {
        // Parse pattern -> expression
        Pattern* pattern = parse_pattern(parser);
        consume(parser, TOKEN_ARROW, "Expected '->' after pattern");

        Expr* body = parse_expression(parser);

        cases = realloc(cases, sizeof(MatchCase) * (case_count + 1));
        cases[case_count].pattern = pattern;
        cases[case_count].body = body;
        case_count++;
    }

    consume(parser, TOKEN_RBRACE, "Expected '}' after match cases");

    return create_match_stmt(scrutinee, cases, case_count);
}

// Parse expression
Expr* parse_expression(Parser* parser) {
    return parse_primary(parser); // Simplified - just parse primary expressions
}

// Parse primary expression
Expr* parse_primary(Parser* parser) {
    if (match(parser, TOKEN_INT_LITERAL)) {
        LiteralValue value = { .int_val = parser->previous_token.literal.int_val };
        return create_literal_expr(LIT_INT, value);
    } else if (match(parser, TOKEN_STRING_LITERAL)) {
        LiteralValue value = { .string_val = parser->previous_token.literal.string_val };
        return create_literal_expr(LIT_STRING, value);
    } else if (match(parser, TOKEN_IDENTIFIER)) {
        char* name = strndup(parser->previous_token.lexeme, parser->previous_token.length);
        return create_identifier_expr(name);
    } else if (match(parser, TOKEN_TRUE)) {
        LiteralValue value = { .bool_val = true };
        return create_literal_expr(LIT_BOOL, value);
    } else if (match(parser, TOKEN_FALSE)) {
        LiteralValue value = { .bool_val = false };
        return create_literal_expr(LIT_BOOL, value);
    }

    report_syntax_error(E1001_UNEXPECTED_TOKEN, "Expected expression",
                       NULL, parser->current_token.line, parser->current_token.column);
    return NULL;
}

// Parse type
Type* parse_type(Parser* parser) {
    if (match(parser, TOKEN_IDENTIFIER)) {
        char* name = strndup(parser->previous_token.lexeme, parser->previous_token.length);
        return create_named_type(name);
    }

    report_syntax_error(E1001_UNEXPECTED_TOKEN, "Expected type",
                       NULL, parser->current_token.line, parser->current_token.column);
    return NULL;
}

// Parse pattern (simplified)
Pattern* parse_pattern(Parser* parser) {
    if (match(parser, TOKEN_IDENTIFIER)) {
        char* name = strndup(parser->previous_token.lexeme, parser->previous_token.length);
        return create_constructor_pattern(name, NULL, 0);
    }

    report_syntax_error(E1001_UNEXPECTED_TOKEN, "Expected pattern",
                       NULL, parser->current_token.line, parser->current_token.column);
    return NULL;
}
