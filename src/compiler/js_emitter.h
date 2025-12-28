#ifndef MANAKNIGHT_JS_EMITTER_H
#define MANAKNIGHT_JS_EMITTER_H

#include "ast.h"
#include <stdio.h>

// JS Emitter
typedef struct {
    char* buffer;
    size_t buffer_size;
    size_t buffer_capacity;
} JSEmitter;

JSEmitter* js_emitter_create(void);
void js_emitter_free(JSEmitter* emitter);
void js_emitter_emit_program(JSEmitter* emitter, Program* program);
char* js_emitter_get_code(JSEmitter* emitter);

#endif // MANAKNIGHT_JS_EMITTER_H
