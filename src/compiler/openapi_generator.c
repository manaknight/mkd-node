#include "openapi_generator.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define INITIAL_BUFFER_SIZE 1024
#define BUFFER_GROWTH_FACTOR 2

static void openapi_generator_grow_buffer(OpenAPIGenerator* generator, size_t needed) {
    while (generator->buffer_capacity < needed) {
        generator->buffer_capacity *= BUFFER_GROWTH_FACTOR;
    }
    generator->buffer = realloc(generator->buffer, generator->buffer_capacity);
}

static void openapi_generator_append(OpenAPIGenerator* generator, const char* str) {
    size_t len = strlen(str);
    size_t needed = generator->buffer_size + len + 1;

    if (needed > generator->buffer_capacity) {
        openapi_generator_grow_buffer(generator, needed);
    }

    strcpy(generator->buffer + generator->buffer_size, str);
    generator->buffer_size += len;
}

static const char* map_http_method(const char* method) {
    if (strcmp(method, "get") == 0) return "get";
    if (strcmp(method, "post") == 0) return "post";
    if (strcmp(method, "put") == 0) return "put";
    if (strcmp(method, "delete") == 0) return "delete";
    if (strcmp(method, "head") == 0) return "head";
    return method; // fallback
}

static void openapi_generator_generate_paths(OpenAPIGenerator* generator, Program* program) {
    openapi_generator_append(generator, "  \"paths\": {\n");

    int first_path = 1;
    for (size_t i = 0; i < program->module_count; i++) {
        Module* module = program->modules[i];

        for (size_t j = 0; j < module->api_route_count; j++) {
            ApiRoute* route = module->api_routes[j];

            if (!first_path) {
                openapi_generator_append(generator, ",\n");
            }
            first_path = 0;

            openapi_generator_append(generator, "    \"");
            openapi_generator_append(generator, route->path);
            openapi_generator_append(generator, "\": {\n");
            openapi_generator_append(generator, "      \"");
            openapi_generator_append(generator, map_http_method(route->method));
            openapi_generator_append(generator, "\": {\n");
            openapi_generator_append(generator, "        \"responses\": {\n");
            openapi_generator_append(generator, "          \"200\": {\n");
            openapi_generator_append(generator, "            \"description\": \"Successful response\",\n");
            openapi_generator_append(generator, "            \"content\": {\n");
            openapi_generator_append(generator, "              \"application/json\": {\n");
            openapi_generator_append(generator, "                \"schema\": {\n");
            openapi_generator_append(generator, "                  \"type\": \"string\"\n");
            openapi_generator_append(generator, "                }\n");
            openapi_generator_append(generator, "              }\n");
            openapi_generator_append(generator, "            }\n");
            openapi_generator_append(generator, "          }\n");
            openapi_generator_append(generator, "        }\n");
            openapi_generator_append(generator, "      }\n");
            openapi_generator_append(generator, "    }");
        }
    }

    openapi_generator_append(generator, "\n  }\n");
}

OpenAPIGenerator* openapi_generator_create(void) {
    OpenAPIGenerator* generator = calloc(1, sizeof(OpenAPIGenerator));
    if (!generator) return NULL;

    generator->buffer_capacity = INITIAL_BUFFER_SIZE;
    generator->buffer = malloc(generator->buffer_capacity);
    if (!generator->buffer) {
        free(generator);
        return NULL;
    }

    generator->buffer[0] = '\0';
    generator->buffer_size = 0;

    return generator;
}

void openapi_generator_free(OpenAPIGenerator* generator) {
    if (generator) {
        free(generator->buffer);
        free(generator);
    }
}

void openapi_generator_generate(OpenAPIGenerator* generator, Program* program) {
    openapi_generator_append(generator, "{\n");
    openapi_generator_append(generator, "  \"openapi\": \"3.0.0\",\n");
    openapi_generator_append(generator, "  \"info\": {\n");
    openapi_generator_append(generator, "    \"title\": \"Manaknight API\",\n");
    openapi_generator_append(generator, "    \"version\": \"1.0.0\"\n");
    openapi_generator_append(generator, "  },\n");

    openapi_generator_generate_paths(generator, program);

    openapi_generator_append(generator, "}\n");
}

char* openapi_generator_get_json(OpenAPIGenerator* generator) {
    return generator->buffer;
}

