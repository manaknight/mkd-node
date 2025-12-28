### mkd-node / MicroQuickJS repo map (what each file is, where to start, where to extend)

This repository contains **MicroQuickJS (MQuickJS)**: a small-footprint JavaScript engine (ES5-ish + selected ES6 additions) designed for embedded use. It ships with:

- a **REPL / CLI** (`mqjs`) that can run scripts and emit/load **persistent bytecode**
- an **embedding example** (`example`)
- a **ROM-friendly “standard library”** that is **compiled into C data** at build time (not constructed at runtime)

---

### Quick start (build + run + tests)

- **Build**: `make`
- **Run a script**: `./mqjs tests/test_builtin.js`
- **Run the embedding example**: `./example tests/test_rect.js`
- **Run tests**: `make test`
- **Generate bytecode**: `./mqjs -o out.bin your.js`
- **Run bytecode**: `./mqjs -b out.bin`

Key idea: the engine does **not** use `malloc()` internally; you give it a **single memory buffer** and it allocates within it (see `README.md`, `mquickjs.h`).

---

### Where to start (recommended reading order)

- **“How do I run something?”**: `mqjs.c` + `README.md`
- **“How do I embed it in my program?”**: `example.c` + `mquickjs.h`
- **“Where are globals like `Array`, `Math`, `JSON` defined?”**: `mqjs_stdlib.c` (then understand generation via `mquickjs_build.c`)
- **“Where is the engine implemented?”**: `mquickjs.c` (large file; holds parser/compiler/VM/runtime/GC)

---

### Architecture in one page

- **Host programs**
  - `mqjs.c`: REPL/CLI, file loading, bytecode save/load, small host-provided globals (`print`, timers, etc.)
  - `example.c`: embedding sample + custom classes, shows correct GC-safe usage patterns

- **Engine core**
  - `mquickjs.c`: the VM/runtime/compiler/GC in one compilation unit
  - `mquickjs.h`: public C API (values, parse/run, properties, bytecode, GC ref rules)
  - `mquickjs_priv.h`: internal declarations for stdlib/runtime helpers used by the generated stdlib tables
  - `mquickjs_opcode.h`: VM opcode list and encoding formats

- **ROM-friendly standard library**
  - `mqjs_stdlib.c`: “manifest” describing global classes/functions/prototypes using macros from `mquickjs_build.h`
  - `mquickjs_build.c` + `mquickjs_build.h`: host-side generator that turns that manifest into `mqjs_stdlib.h` (a big ROM table) and `mquickjs_atom.h` (atom constants)

- **Support libraries**
  - `dtoa.c/.h`: float↔string (and integer formatting) used by the engine for number conversions
  - `libm.c/.h`: tiny math library (optionally uses softfloat templates)
  - `softfp_template*.h`: floating point emulation templates (used by `libm.c` when configured)
  - `readline*.c/.h`: small line editor + TTY integration used by the REPL
  - `cutils.c/.h`, `list.h`: portable helpers + intrusive list used internally and by the build tool

---

### How stdlib generation works (important for “where do I add X?”)

The “standard library” that `mqjs` uses is **not assembled at runtime**. Instead:

- `mqjs_stdlib.c` describes the global object/classes/prototypes with `JS_PROP_*` / `JS_CLASS_*` macros (from `mquickjs_build.h`)
- the host tool `mqjs_stdlib` (built from `mqjs_stdlib.c` + `mquickjs_build.c`) outputs:
  - **`mqjs_stdlib.h`**: a ROM-friendly table + C-function tables + finalizer table
  - **`mquickjs_atom.h`**: atom definitions used by the engine/compiler
- `mqjs.c` includes the generated `mqjs_stdlib.h` to boot the runtime quickly

Relevant Makefile rules are in `Makefile` (targets `mqjs_stdlib.h` and `mquickjs_atom.h`).

---

### Where to modify / extend (common recipes)

- **Add a new global JS function implemented in C (e.g. `globalThis.foo`)**
  - Implement it in `mqjs.c` (or a new `.c` you compile into `mqjs`)
  - Add a `JS_CFUNC_DEF("foo", argc, js_foo)` entry in `mqjs_stdlib.c`’s `js_global_object[]`
  - Rebuild so `mqjs_stdlib.h` regenerates (via `make`)

- **Add a new class exposed to JS (with methods/getters/finalizer)**
  - Follow the pattern in `example.c` + `example_stdlib.c`
  - Define `JS_CLASS_*` ids + optional finalizers in your host program
  - Create `JSPropDef[]` for prototype / static props, then `JSClassDef` via `JS_CLASS_DEF(...)`
  - Export it from the stdlib manifest (`mqjs_stdlib.c`), or build your own stdlib header the way `example_stdlib.c` does

- **Change/extend built-in behavior (Object/Array/String/RegExp/JSON/etc.)**
  - Most built-ins are implemented as `js_*` functions inside `mquickjs.c`
  - Their internal declarations live in `mquickjs_priv.h` and are referenced by `mqjs_stdlib.c`
  - This is the path for “engine behavior changes” rather than just “add a new host function”

- **Add/modify the CLI / REPL behavior**
  - `mqjs.c` is the main: argument parsing, `-e`, `-i`, `-o`, `-b`, timers, logging, REPL loop
  - `readline*.c/.h` controls line editing + coloring (see `term_get_color()` in `mqjs.c`)

- **Add/modify bytecode or VM features**
  - Opcodes are declared in `mquickjs_opcode.h`
  - The compiler and VM dispatch live in `mquickjs.c` (you’ll typically touch both)

- **Memory/GC safety when writing C extensions**
  - Read the GC rules in `README.md` and the `JSGCRef` API in `mquickjs.h`
  - Rule of thumb: avoid storing raw `JSValue` temporaries across engine calls; use `JS_PushGCRef` / `JS_AddGCRef` patterns (see `example.c`)

---

### File-by-file reference

#### Top-level docs / build

- **`README.md`**: project overview, REPL options, JS subset notes, C API notes, and bytecode workflow.
- **`TUTORIAL.md`**: (this file) a guided repo map + extension notes.
- **`Changelog`**: human-readable release notes (currently “first public version”).
- **`LICENSE`**: MIT license.
- **`Makefile`**: build graph for:
  - `mqjs` and `example`
  - host-side generators (`mqjs_stdlib`, `example_stdlib`)
  - generated headers (`mqjs_stdlib.h`, `example_stdlib.h`, `mquickjs_atom.h`)
  - test targets (`make test`, `make microbench`, etc.)

#### Host programs

- **`mqjs.c`**: the REPL/CLI executable.
  - **Starting point**: `main()` sets memory size, parses args, creates `JSContext`, runs script/REPL.
  - **Extension points**: add new CLI flags; add host functions like `print`, timers; wire in custom stdlib headers.
  - **Key behaviors**: bytecode compile (`-o`), bytecode load (`-b`), REPL loop + syntax highlighting.

- **`example.c`**: embedding example executable.
  - Demonstrates creating `JSContext`, evaluating a script, printing exceptions.
  - Shows how to implement **custom user classes** (`Rectangle`, `FilledRectangle`), finalizers, and a C closure (`JS_NewCFunctionParams`).

- **`example_stdlib.c`**: builds a custom stdlib header for the example.
  - It defines extra classes (Rectangle/FilledRectangle) and then includes `mqjs_stdlib.c` with `CONFIG_CLASS_EXAMPLE` to reuse the rest of the stdlib.

#### Standard library generation

- **`mqjs_stdlib.c`**: stdlib “manifest” (classes + functions) compiled into `mqjs_stdlib.h`.
  - **Where to extend**: add/remove globals, classes, prototypes by editing the `JSPropDef` / `JSClassDef` structures.
  - **Note**: also declares any extra “special” C functions needed for closures (see `js_c_function_decl[]`).

- **`mquickjs_build.h`**: macro DSL for describing the stdlib (`JS_CFUNC_DEF`, `JS_CLASS_DEF`, `JS_PROP_*`).
  - **Where to extend**: if you need new kinds of property entries in the generated table, this is where the “schema” begins.

- **`mquickjs_build.c`**: host-side code generator.
  - Reads the manifest (arrays of `JSPropDef` / `JSClassDef`) and outputs a ROM table + function/finalizer tables.
  - Produces `mqjs_stdlib.h`/`example_stdlib.h` and atom defines (`mquickjs_atom.h`).

#### Engine core (public + private)

- **`mquickjs.h`**: public API and the `JSValue` representation.
  - **Starting point for embedders**: `JS_NewContext`, `JS_Eval`, `JS_Parse`/`JS_Run`, property getters/setters, string/number conversions, bytecode functions.
  - **Critical extension note**: GC is compacting; use `JSGCRef` helpers for safe references.

- **`mquickjs.c`**: the whole engine implementation (large).
  - Contains: allocator + compacting GC, atoms/strings, objects/properties, parser/compiler, bytecode loader, VM execution, built-in implementations (`js_*`).
  - **Where to extend**: adding language/runtime features typically means editing this file (and sometimes `mquickjs_opcode.h`).

- **`mquickjs_priv.h`**: internal declarations referenced by generated stdlib headers.
  - If you add a new built-in `js_foo_*` function in `mquickjs.c` and want to reference it from `mqjs_stdlib.c`, you’ll usually add its prototype here.

- **`mquickjs_opcode.h`**: opcode + operand format definitions.
  - Used by both the compiler (emitter) and VM (decoder/dispatcher).

#### Support libraries (used by engine and/or tools)

- **`cutils.h` / `cutils.c`**: low-level helpers:
  - string helpers (`pstrcpy`, `pstrcat`, suffix checks)
  - bit ops, endian helpers, float bit-casting
  - UTF-8 encode/decode helpers used by string handling and readline

- **`list.h`**: minimal Linux-klist style intrusive linked list utilities (used in `mquickjs_build.c`).

- **`dtoa.h` / `dtoa.c`**: float formatting/parsing and integer-to-string routines.
  - Used by number formatting (`toString`, `toFixed`, parsing, etc.).

- **`libm.h` / `libm.c`**: tiny math library implementation used by `Math.*` and numeric ops.
  - Can optionally wrap/replace FP ops via softfloat templates (see `USE_SOFTFLOAT` / `CONFIG_SOFTFLOAT` in `Makefile`).

- **`softfp_template.h` / `softfp_template_icvt.h`**: templates for software floating point operations and conversions; included by `libm.c` when softfloat is enabled.

- **`readline.h` / `readline.c`**: small readline-like line editor.
  - Used by the REPL; supports UTF-8 cursor movement and optional syntax coloring hook.

- **`readline_tty.h` / `readline_tty.c`**: terminal I/O backend (POSIX termios + Windows support) for the readline utility.

#### Tests / benchmarks (JavaScript)

All JS tests live in `tests/` and are run via `make test` (see `Makefile`).

- **`tests/test_builtin.js`**: broad coverage of builtins (Object/Array/String/RegExp/JSON/TypedArray/Math) + error position tests.
- **`tests/test_closure.js`**: closure semantics and recursion sanity checks.
- **`tests/test_language.js`**: operator semantics, coercions, labels, `arguments`, etc.
- **`tests/test_loop.js`**: control flow, loops, `switch`, `try/catch/finally`, `for..in`.
- **`tests/test_rect.js`**: exercises the embedding example’s custom classes + C closure.
- **`tests/microbench.js`**: microbenchmark suite (run via `make microbench`).
- **`tests/mandelbrot.js`**: demo program (colored terminal mandelbrot).

---

### Generated files you won’t see until you build

After `make`, you’ll also get (and should treat as generated artifacts):

- **`mqjs_stdlib.h`**: generated ROM stdlib table used by `mqjs`
- **`example_stdlib.h`**: generated ROM stdlib table used by `example`
- **`mquickjs_atom.h`**: generated atom constants used by the engine/compiler

If you edit `mqjs_stdlib.c` / `example_stdlib.c`, rebuilding regenerates these headers automatically.
