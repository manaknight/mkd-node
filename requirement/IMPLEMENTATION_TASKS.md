# Manaknight Implementation Master Plan

This document is the **single source of truth for implementation**. It contains all tasks, dependencies, acceptance criteria, and technical guidance required to build the Manaknight compiler and runtime on top of `mqjs`.

---

## Phase 1: Compiler Scaffolding & Parsing

### Task 1.1: AST Definitions
**Goal**: Define the immutable data structures representing the Manaknight language.
**Dependencies**: None
**Acceptance Criteria**:
- [x] Define all AST nodes from `MANAKNIGHTLANGUAGE.md` (Section 8):
    - **Top-Level**: `Program`, `Module`, `ApiRoute`.
    - **Declarations**: `FunctionDecl`, `TypeDecl`, `EffectDecl`, `ImportDecl`.
    - **Statements**: `Block`, `LetStmt`, `ExprStmt`, `IfStmt`, `MatchStmt`.
    - **Expressions**: `Literal`, `IdentifierExpr`, `CallExpr`, `LambdaExpr`, `IfExpr`, `MatchExpr`, `PipeExpr`.
    - **Patterns**: `ConstructorPattern`, `WildcardPattern`.
    - **Types**: `PrimitiveType`, `NamedType`, `GenericType`, `FunctionType`.
- [x] AST nodes are strictly typed (C structs with unions).
- [x] `Program` node contains `Module[]` and `ApiRoute[]`.

**Implementation Guide**:
```ts
// src/compiler/ast.ts
export type Node = Program | Module | ApiRoute | Decl | Stmt | Expr;

export interface Program { kind: 'Program', modules: Module[], apis: ApiRoute[] }
export interface Module { kind: 'Module', name: string, decls: Decl[] }
export interface ApiRoute { kind: 'Api', method: string, path: string, body: Block }

export type Decl = FunctionDecl | TypeDecl | EffectDecl | ImportDecl;
export interface FunctionDecl {
  kind: 'FunctionDecl';
  name: string;
  params: Param[];
  returnType: Type;
  effects: string[]; // Explicit effects
  body: Block;
}
// ... define LetStmt, IfStmt, MatchStmt, etc.
```

### Task 1.2: Lexer Implementation
**Goal**: Convert source text into a stream of tokens.
**Dependencies**: None
**Acceptance Criteria**:
- [x] Handles all keywords: `function`, `let`, `if`, `match`, `effect`, `api`, `module` etc.
- [x] Handles operators: `|>` (pipe), `->` (arrow), `==` (equality), `!=`, `>=`, `<=`, `+` (string/int).
- [x] Handles literals: Integers (Int64 format), Strings (UTF-8).
- [x] **Comments**: Strips single-line comments (`//`).
- [x] **Path Handling**: API paths (`/users/:id`) must be tokenized correctly (either as a specific token or handled by parser).
- [x] Tracks Line/Column for error reporting.

### Task 1.3: Recursive Descent Parser
**Goal**: Convert Token stream into AST.
**Dependencies**: 1.1, 1.2
**Acceptance Criteria**:
- [x] Parses valid source code into correct AST.
- [x] Parses `api GET /path` syntax correctly (unique to Manaknight).
- [x] Enforces EBNF grammar (e.g., `let` must be followed by `=`).
- [x] Rejects invalid syntax with `E1xxx` error codes.

**Implementation Guide**:
```ts
// src/compiler/parser.ts
class Parser {
  parseModule(): Module {
    this.expect('module');
    const name = this.parseIdentifier();
    this.expect('{');
    const decls = [];
    while (!this.check('}')) {
      decls.push(this.parseDecl());
    }
    this.expect('}');
    return { kind: 'Module', name, decls };
  }

  parseApi(): ApiRoute {
    this.expect('api');
    const method = this.parseHttpMethod(); // GET, POST...
    const path = this.parsePath();         // /users/:id
    this.expect('{');
    const body = this.parseBlock();
    return { kind: 'Api', method, path, body };
  }
}
```

### Task 1.4: Canonical Formatter
**Goal**: Enforce deterministic source formatting (Spec Section 2).
**Dependencies**: 1.3
**Acceptance Criteria**:
- [x] `mkc fmt file.mk` outputs canonically formatted code.
- [x] **Idempotency**: Running format twice produces identical output (`format(format(x)) == format(x)`).

### Task 1.5: Error Catalog
**Goal**: Centralize all error definitions.
**Dependencies**: None
**Acceptance Criteria**:
- [x] Create `src/compiler/errors.h` and `src/compiler/errors.c`.
- [x] Define enum/map for all codes matching Spec Section 10:
    - **E1000-E1999**: Syntax & Parsing (e.g. Unexpected Token).
    - **E2000-E2999**: Type System (e.g. Type Mismatch, Unknown Identifier).
    - **E3000-E3999**: Effect System (e.g. Undeclared Effect, Effect Leakage).
    - **E4000-E4999**: Pattern Matching (e.g. Non-Exhaustive).
    - **E5000-E5999**: Modules (e.g. Circular Dependency).
    - **E6000-E6999**: API Definitions (e.g. Invalid Method).
    - **E7000-E7999**: Runtime (e.g. Invalid Bytecode).
    - **E8000-E8999**: Resource Limits (e.g. Timeout, OOM).
    - **E9000-E9999**: Internal Errors.
- [x] Ensure every compiler phase uses this catalog for consistent reporting.

---

## Phase 2: Semantics & Verification

### Task 2.0: Module Resolution Strategy
**Goal**: Define how `import auth.user` maps to file paths.
**Dependencies**: None
**Acceptance Criteria**:
- [x] Define mapping strategy (e.g., `auth.user` -> `./auth/user.mk`).
- [x] Implement `ModuleResolver` class to locate files.
- [x] **Recursive Parsing**: Parse imported files recursively to build the complete program AST before Type Checking.
- [x] Enforce "No circular dependency" check (E5004).

### Task 2.1: Symbol Table & Scope
**Goal**: Track variable/function declarations and scopes.
**Dependencies**: 1.3
**Acceptance Criteria**:
- [x] Scopes are nested (Block scope inherits from Function scope).
- [x] Shadows are **forbidden** (Error `E2006`).
- [x] **Imports**: Handle aliasing (`import foo as bar`) and name conflict detection.
- [x] Resolves identifiers to their definition type.
- [x] **Prelude**: Automatically import `core.mk` symbols (Option, Result) into the global scope.

### Task 2.2: Type Checker
**Goal**: Verify type safety rules.
**Dependencies**: 2.1
**Acceptance Criteria**:
- [x] `Let`: Infers type of expr.
- [x] `Call`: Verifies arg count and types match signature.
- [x] `If`: Verifies condition is `Bool` and branches unify.
- [x] `Match`: Verifies scrutinee is ADT and branches unify.
- [x] **Generics**: Verify type arguments match constraints (if any) or are valid types (e.g. `Option<Int>`).
- [x] **Comparisons**: Verify `>` / `<` are only used on `Int` and `String`.
- [x] **Control Flow**: Verify all paths return a value (Totality check/E2005).
- [x] Returns `E2xxx` errors on failure.

### Task 2.3: Effect Analyzer
**Goal**: Enforce capability safety.
**Dependencies**: 2.2
**Acceptance Criteria**:
- [ ] Infers effects for every expression.
- [ ] Verifies `inferred_effects ⊆ declared_effects`.
- [ ] Pure functions cannot call effectful functions (`E3002`).
- [ ] Lambdas must be pure (`E3004`).

**Implementation Guide**:
```ts
function checkEffects(expr: Expr): Set<string> {
  if (expr.kind === 'Call') {
    return expr.callee.effects;
  }
  if (expr.kind === 'Block') {
    return union(expr.stmts.map(checkEffects));
  }
  return new Set(); // Literals are pure
}
```

### Task 2.4: Exhaustiveness Checker
**Goal**: Ensure `match` handles all ADT cases.
**Dependencies**: 2.2
**Acceptance Criteria**:
- [ ] Compile error `E4001` if a constructor is missing.
- [ ] Compile error `E4003` if a pattern is duplicated.
- [ ] Supports `_` wildcard.

---

## Phase 3: Lowering (Translation to JS)

### Task 3.0: Intrinsic Mapping Strategy
**Goal**: Define how core types map to JS/C.
**Dependencies**: None
**Acceptance Criteria**:
- [ ] **Int64**: Prefer C-Host implementation if `BigInt` is missing in `mqjs` (for performance).
- [ ] **String**: JS String (UTF-16) vs Uint8Array (UTF-8). Document decision.
- [ ] Document these decisions as they affect Lowering.

### Task 3.1: IR Lowering (Logic)
**Goal**: Transform Manaknight AST to Safe JS Subset AST.
**Dependencies**: 2.3, 3.0
**Acceptance Criteria**:
- [ ] `function` -> JS `function`.
- [ ] `let x = y` -> `const x = y`.
- [ ] `match` -> `if (x.tag === '...')`.
- [ ] **Pipeline**: Desugar `a |> f` into `f(a)`.
- [ ] **ADT Constructors**: Transform constructor calls (e.g., `some(5)`) into tagged objects (`{ tag: 'some', value: 5 }`).
- [ ] `if` expression -> wrapped IIFE or hoisted var.
- [ ] **API Routes**: Transform `api GET /path` into runtime registration calls (e.g., `__router.register("GET", "/path", fn)`).
- [ ] **Recursion**: Ensure tail calls (or loops if optimized) are used to prevent stack overflow if possible, though TCO is not guaranteed by spec.

### Task 3.2: Effect Injection Lowering
**Goal**: Pass capabilities explicitly at runtime.
**Dependencies**: 3.1
**Acceptance Criteria**:
- [ ] Effectful functions receive extra `__effects` argument.
- [ ] Call sites pass `__effects` through.
- [ ] `time.now()` lowers to `__effects.time.now()`.

### Task 3.3: JS Emitter
**Goal**: Generate string output for MicroQuickJS.
**Dependencies**: 3.2
**Acceptance Criteria**:
- [ ] Output is valid ES5/ES6.
- [ ] No forbidden constructs (`eval`, `with`, `class`).
- [ ] Includes `"use strict"`.
- [ ] **Version Check**: Embeds a check for Runtime Version compatibility (Spec Section 10).
- [ ] **Metadata**: Emit `export const __meta = { version: '...', effects: [...] }` for Host consumption.
- [ ] **External Imports**: Emit JS `import` statements for dependencies (don't bundle) to allow Host Module Loading.

### Task 3.4: OpenAPI Generator
**Goal**: Generate OpenAPI spec for declared APIs at compile time.
**Dependencies**: 1.3, 2.3
**Acceptance Criteria**:
- [ ] Generates valid OpenAPI 3.0 JSON.
- [ ] Maps Manaknight types to OpenAPI Schemas (`Int` -> `integer`, `String` -> `string`, etc.).
- [ ] Includes all declared `api` routes with correct Methods and Paths.
- [ ] Generated as a separate artifact (e.g., `openapi.json`).

---

## Phase 4: Standard Library & Runtime

### Task 4.0: Stdlib Type Definitions (.mk)
**Goal**: Create source definitions for Type Checking.
**Dependencies**: 1.3
**Acceptance Criteria**:
- [ ] `core.mk`: Defines `Option`, `Result`, `List`, `Map`, `Json`.
- [ ] `math.mk`: Defines `add`, `sub`, `mul`, `div`, etc.
- [ ] `effects.mk`: Defines interfaces for `time`, `random`, `http`, `log`.
- [ ] Compiler loads these at startup.

### Task 4.1: Stdlib Core (Implementation)
**Goal**: Implement Tier-0 types in JS/Manaknight.
**Dependencies**: 3.3, 3.0
**Acceptance Criteria**:
- [ ] **Math**: Implement Int64 with overflow checks triggering Host Trap.
    - `add`, `sub`, `mul`, `div`, `mod`, `abs`, `min`, `max`, `clamp`.
- [ ] **String**: Implement with UTF-8 semantics.
    - `length`, `isEmpty`, `concat`, `split`, `join`, `contains`, `startsWith`, `endsWith`, `toUpperAscii`, `toLowerAscii`.
- [ ] **List**: Implement core ops using iteration to avoid stack overflow.
    - `map`, `flatMap`, `filter`, `fold`, `foldRight`, `length`, `reverse`, `take`, `drop`, `find`, `all`, `any`.
- [ ] **Map**: Implement with value semantics and deterministic iteration.
    - `empty`, `get`, `set`, `remove`, `containsKey`, `keys`, `values`, `merge`.
- [ ] **Json**: Implement boundary checking.
    - `encode`, `decode`, `get`, `getString`, `getInt`, `getObject`, `getArray`.
- [ ] **Option/Result**: Implement monadic helpers (`map`, `flatMap`, `unwrapOr`).

### Task 4.2: Host Runtime (C)
**Goal**: Extend `mqjs` to boot the environment.
**Dependencies**: Existing `mqjs` repo
**Acceptance Criteria**:
- [ ] Creates a JS Context.
- [ ] Loads the compiled bytecode.
- [ ] **Resource Limits**: Configure `JS_SetMemoryLimit`, `JS_SetInterruptHandler` (for CPU/Timeout), and handle Allocation limits.
- [ ] **Module Loader**: Implement `JS_SetModuleLoaderFunc` to resolve and load dependency `.bin` files from disk (Spec Section 5.5).
- [ ] **Router**: Implement logic to map incoming HTTP requests (Method/Path) to the correct Bytecode file (Spec Section 5.5).
- [ ] **Security**: Read the bytecode's Effect Manifest (`__meta`) and inject *only* the declared effects.
- [ ] Constructs the `__effects` object with native C function bindings (using `JS_NewObject`, `JS_SetPropertyStr` API).

### Task 4.3: Effect Handlers (C)
**Goal**: Implement the "dirty" side of effects.
**Dependencies**: 4.2
**Acceptance Criteria**:
- [ ] `time` effect: `now`, `unixMillis`.
- [ ] `random` effect: `int`, `bytes`.
- [ ] `log` effect: `info`, `warn`, `error`.
- [ ] `http` effect: `getHeader`, `setHeader`, `json`, `text`, `status`.

---

## Phase 5: Verification & Delivery

### Task 5.1: Compiler CLI (mkc)
**Goal**: User-facing tool.
**Dependencies**: 3.3, 5.0 (Stdlib)
**Acceptance Criteria**:
- [ ] `mkc input.mk -o output.bin` works.
- [ ] `mkc --openapi output.json` works (runs Task 3.4).
- [ ] Implements **File System Resolution** for imports.
- [ ] Invokes internal JS emitter -> invokes `mqjs -o` (bytecode flag) -> outputs bytecode.
- [ ] **Error Reporting**: Outputs errors in JSON/Format defined in Spec Section 10.
- [ ] Exits with non-zero code on error.

### Task 5.2: End-to-End Test Suite
**Goal**: Verify the whole chain.
**Dependencies**: 5.1
**Acceptance Criteria**:
- [ ] Harness runs: Source -> `mkc` -> Bytecode -> `mqjs` -> Output.
- [ ] Includes "The Rect Test" (from Tutorial).
- [ ] Verifies all Tier-0 libraries work in the VM.
- [ ] Verifies resource limits (infinite recursion throws E8003, memory limit throws E8002).
- [ ] Verifies overflow checks (INT_MAX + 1 throws runtime error).
