#include "parser.h"
#include "ast.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// Parser functions
Parser* parser_create(Lexer* lexer, const char* filename) {
    Parser* parser = calloc(1, sizeof(Parser));
    if (!parser) return NULL;

    parser->lexer = lexer;
    parser->filename = strdup(filename);
    parser->current_token = lexer_next_token(lexer);

    return parser;
}

void parser_free(Parser* parser) {
    if (!parser) return;
    free(parser->filename);
    free(parser);
}

// Forward declarations
ApiRoute* parser_parse_api_route(Parser* parser);

Program* parser_parse_program(Parser* parser) {
    Program* program = ast_create_program();

    // Create a default module for now
    Module* module = calloc(1, sizeof(Module));
    if (!module) return program;

    module->base.type = NODE_MODULE;
    module->name = strdup("main");
    module->path = strdup("main.mk");

    // Parse declarations
    while (parser->current_token.type != TOK_EOF) {
        if (parser->current_token.type == TOK_FN) {
            FunctionDecl* func = parser_parse_function(parser);
            if (func) {
                module->functions = realloc(module->functions,
                    sizeof(FunctionDecl*) * (module->function_count + 1));
                module->functions[module->function_count] = func;
                module->function_count++;
            }
        } else if (parser->current_token.type == TOK_API) {
            ApiRoute* route = parser_parse_api_route(parser);
            if (route) {
                module->api_routes = realloc(module->api_routes,
                    sizeof(ApiRoute*) * (module->api_route_count + 1));
                module->api_routes[module->api_route_count] = route;
                module->api_route_count++;
            }
        } else {
            // Skip unknown tokens and comments
            parser->current_token = lexer_next_token(parser->lexer);
        }
    }

    // Add module to program
    program->modules = realloc(program->modules,
        sizeof(Module*) * (program->module_count + 1));
    program->modules[program->module_count] = module;
    program->module_count++;

    return program;
}

ApiRoute* parser_parse_api_route(Parser* parser) {
    // Consume 'api'
    parser->current_token = lexer_next_token(parser->lexer);

    // Expect method (like 'get', 'post', etc.)
    if (parser->current_token.type != TOK_IDENTIFIER &&
        parser->current_token.type != TOK_GET) {
        return NULL;
    }

    ApiRoute* route = calloc(1, sizeof(ApiRoute));
    if (!route) return NULL;

    route->base.type = NODE_API_ROUTE;
    route->method = strdup(parser->current_token.text);

    // Consume method
    parser->current_token = lexer_next_token(parser->lexer);

    // Expect string literal for path
    if (parser->current_token.type != TOK_STRING_LITERAL) {
        free(route->method);
        free(route);
        return NULL;
    }

    route->path = strdup(parser->current_token.text);

    // Consume path
    parser->current_token = lexer_next_token(parser->lexer);

    // Expect '('
    if (parser->current_token.type != TOK_LPAREN) {
        free(route->method);
        free(route->path);
        free(route);
        return NULL;
    }

    // Skip parameters for now - expect ')'
    parser->current_token = lexer_next_token(parser->lexer);
    if (parser->current_token.type != TOK_RPAREN) {
        free(route->method);
        free(route->path);
        free(route);
        return NULL;
    }

    // Consume ')'
    parser->current_token = lexer_next_token(parser->lexer);

    // Expect '->'
    if (parser->current_token.type != TOK_ARROW) {
        free(route->method);
        free(route->path);
        free(route);
        return NULL;
    }

    // Consume '->'
    parser->current_token = lexer_next_token(parser->lexer);

    // Skip return type for now
    while (parser->current_token.type != TOK_LBRACE &&
           parser->current_token.type != TOK_EOF) {
        parser->current_token = lexer_next_token(parser->lexer);
    }

    // Expect '{'
    if (parser->current_token.type != TOK_LBRACE) {
        free(route->method);
        free(route->path);
        free(route);
        return NULL;
    }

    // Create a function declaration for the handler
    FunctionDecl* handler = calloc(1, sizeof(FunctionDecl));
    if (!handler) {
        free(route->method);
        free(route->path);
        free(route);
        return NULL;
    }

    handler->base.type = NODE_FUNCTION_DECL;
    handler->name = strdup("handler"); // Internal name for the API handler
    handler->param_names = NULL;
    handler->param_count = 0;
    handler->effect_names = NULL;
    handler->effect_count = 0;
    handler->return_type = NULL;

    // Parse the handler body
    handler->body = parser_parse_block(parser);
    route->handler = handler;

    return route;
}

FunctionDecl* parser_parse_function(Parser* parser) {
    // Consume 'fn'
    parser->current_token = lexer_next_token(parser->lexer);

    // Expect identifier (function name)
    if (parser->current_token.type != TOK_IDENTIFIER) {
        return NULL;
    }

    FunctionDecl* func = calloc(1, sizeof(FunctionDecl));
    if (!func) return NULL;

    func->base.type = NODE_FUNCTION_DECL;
    func->name = strdup(parser->current_token.text);

    // Consume function name
    parser->current_token = lexer_next_token(parser->lexer);

    // Expect '('
    if (parser->current_token.type != TOK_LPAREN) {
        free(func->name);
        free(func);
        return NULL;
    }

    // Skip parameters for now - expect ')'
    parser->current_token = lexer_next_token(parser->lexer);
    if (parser->current_token.type != TOK_RPAREN) {
        free(func->name);
        free(func);
        return NULL;
    }

    // Consume ')'
    parser->current_token = lexer_next_token(parser->lexer);

    // Expect '->'
    if (parser->current_token.type != TOK_ARROW) {
        free(func->name);
        free(func);
        return NULL;
    }

    // Consume '->'
    parser->current_token = lexer_next_token(parser->lexer);

    // Skip return type for now
    while (parser->current_token.type != TOK_LBRACE &&
           parser->current_token.type != TOK_EOF) {
        parser->current_token = lexer_next_token(parser->lexer);
    }

    // Expect '{'
    if (parser->current_token.type != TOK_LBRACE) {
        free(func->name);
        free(func);
        return NULL;
    }

    // Parse block
    func->body = parser_parse_block(parser);

    return func;
}

Block* parser_parse_block(Parser* parser) {
    Block* block = calloc(1, sizeof(Block));
    if (!block) return NULL;

    block->base.type = NODE_BLOCK;

    // Consume '{'
    parser->current_token = lexer_next_token(parser->lexer);

    // For now, just look for a string literal followed by '}'
    if (parser->current_token.type == TOK_STRING_LITERAL) {
        // Create a literal expression
        Literal* literal = calloc(1, sizeof(Literal));
        if (literal) {
            literal->base.type = NODE_LITERAL;
            literal->kind = LIT_STRING;
            literal->value.string_val = strdup(parser->current_token.text);

            block->result_expr = literal;
        }

        // Consume the string
        parser->current_token = lexer_next_token(parser->lexer);

        // Expect '}'
        if (parser->current_token.type == TOK_RBRACE) {
            parser->current_token = lexer_next_token(parser->lexer);
        }
    }

    return block;
}

void parser_skip_to_next_declaration(Parser* parser) {
    // Skip the entire API declaration by counting braces
    int brace_count = 0;

    while (parser->current_token.type != TOK_EOF) {
        if (parser->current_token.type == TOK_LBRACE) {
            brace_count++;
        } else if (parser->current_token.type == TOK_RBRACE) {
            brace_count--;
            if (brace_count == 0) {
                // Found the end of the API declaration
                parser->current_token = lexer_next_token(parser->lexer);
                break;
            }
        }
        parser->current_token = lexer_next_token(parser->lexer);
    }
}

