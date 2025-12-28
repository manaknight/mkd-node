# Manaknight Implementation Tasks

This task list is derived from the **Manaknight Language Specification** (`requirement/MANAKNIGHTLANGUAGE.md`).
It breaks down the implementation into actionable phases for an AI or developer.

## Phase 1: Compiler Scaffolding
- [ ] **Setup Project**: Initialize TypeScript project for the compiler.
- [ ] **AST Definitions**: Implement the AST types defined in Spec Section 8 (Program, Module, FunctionDecl, etc.).
- [ ] **Lexer**: Implement lexer for the EBNF grammar (Spec Section 2).
- [ ] **Parser**: Implement recursive descent parser for the AST.
- [ ] **Formatter**: Implement canonical formatter (Spec Section 2).

## Phase 2: Semantics & Type System
- [ ] **Type Checker**: Implement the rules from Spec Section 8 (Let, Call, If, Match).
- [ ] **Effect Analyzer**: Implement effect inference and checking (inferred ⊆ declared).
- [ ] **Exhaustiveness**: Implement check for ADT pattern matching.

## Phase 3: Lowering & Codegen
- [ ] **Lowering (IR)**: Implement transformations to JS Subset (Spec Section 8).
  - [ ] Functions & Let bindings
  - [ ] Pattern Matching to `if/else`
  - [ ] Effect injection (`__effects` param)
- [ ] **JS Emitter**: Generate strict ES5/ES6 code compatible with MicroQuickJS.
- [ ] **Source Maps**: Implement source map generation for Debug builds (Spec Section 10).

## Phase 4: Runtime & Stdlib
- [ ] **Stdlib Implementation**: Implement Tier-0 libraries (Spec Section 11).
  - [ ] `Core` (Option, Result, List, Map)
  - [ ] `Math` (Int64 semantics)
  - [ ] `String` (UTF-8 immutable)
- [ ] **Runtime Host**: Extend `mqjs.c` to support Effect Injection.
- [ ] **Effect Handlers**: Implement host-side `time`, `random`, `log`, `http`.

## Phase 5: Verification
- [ ] **Test Suite**: Create tests for all Error Codes (E1001-E9002).
- [ ] **Reproducibility**: Verify identical bytecode generation.
