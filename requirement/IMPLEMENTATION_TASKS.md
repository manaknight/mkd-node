# Manaknight Implementation Master Plan

This document is the **single source of truth for implementation**. It contains all tasks, dependencies, acceptance criteria, and technical guidance required to build the Manaknight compiler and runtime on top of `mqjs`.

---

## Phase 1: Compiler Scaffolding & Parsing

### Task 1.1: AST Definitions
**Goal**: Define the immutable data structures representing the Manaknight language.
**Dependencies**: None
**Acceptance Criteria**:
- [ ] All AST nodes defined in `MANAKNIGHTLANGUAGE.md` (Section 8) are present.
- [ ] AST nodes are strictly typed.
- [ ] `Program` node contains `Module[]` and `ApiRoute[]`.
- [ ] `Module` contains `Declaration[]` (Function, Type, Effect, Import).

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
- [ ] Handles all keywords: `function`, `let`, `if`, `match`, `effect`, `api`, `module` etc.
- [ ] Handles operators: `|>` (pipe), `->` (arrow), `==` (equality).
- [ ] Handles literals: Integers (Int64 format), Strings (UTF-8).
- [ ] Tracks Line/Column for error reporting.

### Task 1.3: Recursive Descent Parser
**Goal**: Convert Token stream into AST.
**Dependencies**: 1.1, 1.2
**Acceptance Criteria**:
- [ ] Parses valid source code into correct AST.
- [ ] Parses `api GET /path` syntax correctly (unique to Manaknight).
- [ ] Enforces EBNF grammar (e.g., `let` must be followed by `=`).
- [ ] Rejects invalid syntax with `E1xxx` error codes.

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

---

## Phase 2: Semantics & Verification

### Task 2.1: Symbol Table & Scope
**Goal**: Track variable/function declarations and scopes.
**Dependencies**: 1.3
**Acceptance Criteria**:
- [ ] Scopes are nested (Block scope inherits from Function scope).
- [ ] Shadows are **forbidden** (Error `E2006`).
- [ ] Resolves identifiers to their definition type.

### Task 2.2: Type Checker
**Goal**: Verify type safety rules.
**Dependencies**: 2.1
**Acceptance Criteria**:
- [ ] `Let`: Infers type of expr.
- [ ] `Call`: Verifies arg count and types match signature.
- [ ] `If`: Verifies condition is `Bool` and branches unify.
- [ ] `Match`: Verifies scrutinee is ADT and branches unify.
- [ ] Returns `E2xxx` errors on failure.

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
- [ ] Decide mapping for `Int`: `BigInt` (if supported) or Library Class.
- [ ] Decide mapping for `String`: JS String (UTF-16) vs Uint8Array (UTF-8).
- [ ] Document this decision as it affects Lowering.

### Task 3.1: IR Lowering (Logic)
**Goal**: Transform Manaknight AST to Safe JS Subset AST.
**Dependencies**: 2.3, 3.0
**Acceptance Criteria**:
- [ ] `function` -> JS `function`.
- [ ] `let x = y` -> `const x = y`.
- [ ] `match` -> `if (x.tag === '...')`.
- [ ] `if` expression -> wrapped IIFE or hoisted var.
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

---

## Phase 4: Standard Library & Runtime

### Task 4.0: Stdlib Type Definitions (.mk)
**Goal**: Create source definitions for Type Checking.
**Dependencies**: 1.3
**Acceptance Criteria**:
- [ ] `core.mk`: Defines `Option`, `Result`, `List`, `Map`.
- [ ] `math.mk`: Defines `add`, `sub`, etc.
- [ ] `effects.mk`: Defines interfaces for `time`, `random`, `http`, `log`.
- [ ] Compiler loads these at startup.

### Task 4.1: Stdlib Core (Implementation)
**Goal**: Implement Tier-0 types in JS/Manaknight.
**Dependencies**: 3.3
**Acceptance Criteria**:
- [ ] **Critical Check**: Verify if target `mqjs` supports `BigInt`. If not, implement `Int64` emulation.
- [ ] **Critical Check**: Verify String UTF-8 behavior. Implement wrapper if needed to guarantee `length` = codepoints.
- [ ] `List` and `Map` implementation (structural equality).

### Task 4.2: Host Runtime (C)
**Goal**: Extend `mqjs` to boot the environment.
**Dependencies**: Existing `mqjs` repo
**Acceptance Criteria**:
- [ ] Creates a JS Context.
- [ ] Loads the compiled bytecode.
- [ ] Constructs the `__effects` object with native C function bindings.

### Task 4.3: Effect Handlers (C)
**Goal**: Implement the "dirty" side of effects.
**Dependencies**: 4.2
**Acceptance Criteria**:
- [ ] `time` effect: Binds to C `gettimeofday`.
- [ ] `random` effect: Binds to C CSPRNG.
- [ ] `log` effect: Binds to C `printf` (structured).
- [ ] `http` effect: Binds to simple HTTP client/server stub.

---

## Phase 5: Verification & Delivery

### Task 5.1: Compiler CLI (mkc)
**Goal**: User-facing tool.
**Dependencies**: 3.3
**Acceptance Criteria**:
- [ ] `mkc input.mk -o output.bin` works.
- [ ] Invokes internal JS emitter -> invokes `mqjs -c` -> outputs bytecode.
- [ ] Exits with non-zero code on error.

### Task 5.2: End-to-End Test Suite
**Goal**: Verify the whole chain.
**Dependencies**: 5.1
**Acceptance Criteria**:
- [ ] Harness runs: Source -> `mkc` -> Bytecode -> `mqjs` -> Output.
- [ ] Includes "The Rect Test" (from Tutorial).
- [ ] Verifies all Tier-0 libraries work in the VM.
