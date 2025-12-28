#ifndef MANAKNIGHT_OPENAPI_GENERATOR_H
#define MANAKNIGHT_OPENAPI_GENERATOR_H

#include "ast.h"
#include <stdio.h>

// OpenAPI Generator
typedef struct {
    char* buffer;
    size_t buffer_size;
    size_t buffer_capacity;
} OpenAPIGenerator;

OpenAPIGenerator* openapi_generator_create(void);
void openapi_generator_free(OpenAPIGenerator* generator);
void openapi_generator_generate(OpenAPIGenerator* generator, Program* program);
char* openapi_generator_get_json(OpenAPIGenerator* generator);

#endif // MANAKNIGHT_OPENAPI_GENERATOR_H
