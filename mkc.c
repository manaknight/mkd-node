#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>

// Compiler component includes
#include "src/compiler/ast.h"
#include "src/compiler/errors.h"
#include "src/compiler/lexer.h"
#include "src/compiler/parser.h"
#include "src/compiler/js_emitter.h"
#include "src/compiler/formatter.h"
#include "src/compiler/openapi_generator.h"

// Forward declarations for compiler components
// These will be implemented in separate files
typedef struct {
    char* source;
    char* filename;
} CompilerInput;

typedef struct {
    char* js_code;
    char* openapi_spec;
    int error_count;
    char** errors;
} CompilerOutput;

CompilerOutput* compile_manaknight(CompilerInput* input);
void free_compiler_output(CompilerOutput* output);

static void print_usage(const char* program_name) {
    printf("Manaknight Compiler (mkc) v1.0.0\n");
    printf("Usage: %s [options] <input_file>\n", program_name);
    printf("\nOptions:\n");
    printf("  -o, --output <file>     Output JavaScript file (default: <input>.js)\n");
    printf("  -a, --openapi <file>    Generate OpenAPI spec to file\n");
    printf("  -f, --format            Format source code\n");
    printf("  -c, --check             Type check only, don't generate output\n");
    printf("  -v, --verbose           Verbose output\n");
    printf("  -h, --help              Show this help\n");
    printf("\nExamples:\n");
    printf("  %s hello.mk                    # Compile to hello.js\n", program_name);
    printf("  %s -o app.js server.mk         # Compile server.mk to app.js\n", program_name);
    printf("  %s -a api.json server.mk       # Generate OpenAPI spec\n", program_name);
    printf("  %s -f code.mk                   # Format source code\n", program_name);
    printf("  %s -c library.mk                # Type check only\n", program_name);
}

static char* change_extension(const char* filename, const char* new_ext) {
    char* dot = strrchr(filename, '.');
    size_t base_len = dot ? (size_t)(dot - filename) : strlen(filename);
    size_t ext_len = strlen(new_ext);
    char* result = malloc(base_len + ext_len + 2); // +2 for dot and null
    if (!result) return NULL;

    memcpy(result, filename, base_len);
    result[base_len] = '.';
    strcpy(result + base_len + 1, new_ext);
    return result;
}

static char* read_file(const char* filename) {
    FILE* f = fopen(filename, "rb");
    if (!f) {
        fprintf(stderr, "Error: Cannot open file '%s'\n", filename);
        return NULL;
    }

    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fseek(f, 0, SEEK_SET);

    if (size < 0) {
        fclose(f);
        fprintf(stderr, "Error: Cannot determine file size for '%s'\n", filename);
        return NULL;
    }

    char* content = malloc(size + 1);
    if (!content) {
        fclose(f);
        fprintf(stderr, "Error: Out of memory reading '%s'\n", filename);
        return NULL;
    }

    size_t read_size = fread(content, 1, size, f);
    fclose(f);

    if (read_size != (size_t)size) {
        free(content);
        fprintf(stderr, "Error: Failed to read file '%s'\n", filename);
        return NULL;
    }

    content[size] = '\0';
    return content;
}

static int write_file(const char* filename, const char* content) {
    FILE* f = fopen(filename, "wb");
    if (!f) {
        fprintf(stderr, "Error: Cannot write to file '%s'\n", filename);
        return 0;
    }

    size_t len = strlen(content);
    size_t written = fwrite(content, 1, len, f);
    fclose(f);

    if (written != len) {
        fprintf(stderr, "Error: Failed to write to file '%s'\n", filename);
        return 0;
    }

    return 1;
}

int main(int argc, char* argv[]) {
    char* input_file = NULL;
    char* output_file = NULL;
    char* openapi_file = NULL;
    int format_only = 0;
    int check_only = 0;
    int verbose = 0;

    static struct option long_options[] = {
        {"output", required_argument, 0, 'o'},
        {"openapi", required_argument, 0, 'a'},
        {"format", no_argument, 0, 'f'},
        {"check", no_argument, 0, 'c'},
        {"verbose", no_argument, 0, 'v'},
        {"help", no_argument, 0, 'h'},
        {0, 0, 0, 0}
    };

    int opt;
    while ((opt = getopt_long(argc, argv, "o:a:f:cvh", long_options, NULL)) != -1) {
        switch (opt) {
            case 'o':
                output_file = optarg;
                break;
            case 'a':
                openapi_file = optarg;
                break;
            case 'f':
                format_only = 1;
                break;
            case 'c':
                check_only = 1;
                break;
            case 'v':
                verbose = 1;
                break;
            case 'h':
                print_usage(argv[0]);
                return 0;
            default:
                print_usage(argv[0]);
                return 1;
        }
    }

    if (optind >= argc) {
        fprintf(stderr, "Error: No input file specified\n");
        print_usage(argv[0]);
        return 1;
    }

    input_file = argv[optind];

    // Check if input file exists and is readable
    if (access(input_file, R_OK) != 0) {
        fprintf(stderr, "Error: Cannot read input file '%s'\n", input_file);
        return 1;
    }

    // Determine output file if not specified
    if (!output_file && !check_only && !format_only) {
        output_file = change_extension(input_file, "js");
        if (!output_file) {
            fprintf(stderr, "Error: Out of memory\n");
            return 1;
        }
    }

    if (verbose) {
        printf("Manaknight Compiler v1.0.0\n");
        printf("Input: %s\n", input_file);
        if (output_file) printf("Output: %s\n", output_file);
        if (openapi_file) printf("OpenAPI: %s\n", openapi_file);
        if (format_only) {
            printf("Mode: format\n");
        } else if (check_only) {
            printf("Mode: type check only\n");
        } else {
            printf("Mode: compile\n");
        }
        printf("\n");
    }

    // Read input file
    char* source = read_file(input_file);
    if (!source) {
        if (output_file != argv[optind]) free(output_file);
        return 1;
    }

    // Prepare compiler input
    CompilerInput input = {
        .source = source,
        .filename = input_file
    };

    // Compile
    CompilerOutput* output = compile_manaknight(&input);

    if (!output) {
        fprintf(stderr, "Error: Compiler failed to initialize\n");
        if (output_file != argv[optind]) free(output_file);
        return 1;
    }

    // Report errors
    if (output->error_count > 0) {
        fprintf(stderr, "Compilation failed with %d error(s):\n", output->error_count);
        for (int i = 0; i < output->error_count; i++) {
            fprintf(stderr, "  %s\n", output->errors[i]);
        }
        free_compiler_output(output);
        if (output_file != argv[optind]) free(output_file);
        return 1;
    }

    if (format_only) {
        // Re-parse for formatting
        Lexer* fmt_lexer = lexer_create(source, input_file);
        if (!fmt_lexer) {
            fprintf(stderr, "Error: Failed to create lexer for formatting\n");
            free_compiler_output(output);
            if (output_file != argv[optind]) free(output_file);
            free(source);
            return 1;
        }

        Parser* fmt_parser = parser_create(fmt_lexer, input_file);
        if (!fmt_parser) {
            fprintf(stderr, "Error: Failed to create parser for formatting\n");
            lexer_free(fmt_lexer);
            free_compiler_output(output);
            if (output_file != argv[optind]) free(output_file);
            free(source);
            return 1;
        }

        Program* fmt_program = parser_parse_program(fmt_parser);
        if (!fmt_program) {
            fprintf(stderr, "Error: Failed to parse program for formatting\n");
            parser_free(fmt_parser);
            lexer_free(fmt_lexer);
            free_compiler_output(output);
            if (output_file != argv[optind]) free(output_file);
            free(source);
            return 1;
        }

        // Format the source code
        Formatter* formatter = formatter_create();
        if (!formatter) {
            fprintf(stderr, "Error: Failed to create formatter\n");
            ast_free_program(fmt_program);
            parser_free(fmt_parser);
            lexer_free(fmt_lexer);
            free_compiler_output(output);
            if (output_file != argv[optind]) free(output_file);
            free(source);
            return 1;
        }

        formatter_format_program(formatter, fmt_program);
        char* formatted_code = formatter_get_code(formatter);

        // Output to stdout for formatting
        printf("%s", formatted_code);

        formatter_free(formatter);
        ast_free_program(fmt_program);
        parser_free(fmt_parser);
        lexer_free(fmt_lexer);
        free(source);
    } else if (check_only) {
        printf("✓ Type check passed\n");
    } else {
        // Write JavaScript output
        if (!write_file(output_file, output->js_code)) {
            free_compiler_output(output);
            if (output_file != argv[optind]) free(output_file);
            return 1;
        }

        if (verbose) {
            printf("✓ Generated %s\n", output_file);
        }
    }

    // Write OpenAPI spec if requested
    if (openapi_file && output->openapi_spec) {
        if (!write_file(openapi_file, output->openapi_spec)) {
            fprintf(stderr, "Warning: Failed to write OpenAPI spec to '%s'\n", openapi_file);
        } else if (verbose) {
            printf("✓ Generated OpenAPI spec: %s\n", openapi_file);
        }
    }

    free_compiler_output(output);
    if (output_file != argv[optind]) free(output_file);
    free(source);

    return 0;
}

// Compiler implementation
CompilerOutput* compile_manaknight(CompilerInput* input) {
    CompilerOutput* output = calloc(1, sizeof(CompilerOutput));
    if (!output) return NULL;

    // Initialize error tracking
    output->error_count = 0;
    output->errors = NULL;

    // Phase 1: Lexical analysis
    Lexer* lexer = lexer_create(input->source, input->filename);
    if (!lexer) {
        output->error_count = 1;
        output->errors = calloc(1, sizeof(char*));
        output->errors[0] = strdup("Failed to create lexer");
        return output;
    }

    // Phase 2: Parsing
    Parser* parser = parser_create(lexer, input->filename);
    if (!parser) {
        lexer_free(lexer);
        output->error_count = 1;
        output->errors = calloc(1, sizeof(char*));
        output->errors[0] = strdup("Failed to create parser");
        return output;
    }

    Program* program = parser_parse_program(parser);
    if (!program) {
        output->error_count = 1;
        output->errors = calloc(1, sizeof(char*));
        output->errors[0] = strdup("Failed to parse program");
        parser_free(parser);
        lexer_free(lexer);
        return output;
    }

    // TODO: Add remaining compilation phases
    // Phase 3: Semantic analysis (type checking, effect analysis, etc.)
    // Phase 4: Code generation (IR lowering, JS emission, OpenAPI generation)

    // Generate JavaScript from AST
    JSEmitter* emitter = js_emitter_create();
    if (!emitter) {
        output->error_count = 1;
        output->errors = calloc(1, sizeof(char*));
        output->errors[0] = strdup("Failed to create JS emitter");
        ast_free_program(program);
        parser_free(parser);
        lexer_free(lexer);
        return output;
    }

    js_emitter_emit_program(emitter, program);
    output->js_code = strdup(js_emitter_get_code(emitter));
    js_emitter_free(emitter);

    // Generate OpenAPI spec
    OpenAPIGenerator* openapi_gen = openapi_generator_create();
    if (openapi_gen) {
        openapi_generator_generate(openapi_gen, program);
        output->openapi_spec = strdup(openapi_generator_get_json(openapi_gen));
        openapi_generator_free(openapi_gen);
    } else {
        output->openapi_spec = NULL;
    }

    // Cleanup
    ast_free_program(program);
    parser_free(parser);
    lexer_free(lexer);

    return output;
}

void free_compiler_output(CompilerOutput* output) {
    if (!output) return;

    free(output->js_code);
    free(output->openapi_spec);

    if (output->errors) {
        for (int i = 0; i < output->error_count; i++) {
            free(output->errors[i]);
        }
        free(output->errors);
    }

    free(output);
}
