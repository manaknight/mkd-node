# How Manaknight Works

## Overview

Manaknight is a **deterministic, functional, capability-based programming language** designed for **auditable systems, APIs, and sandboxed execution**. It combines functional programming purity with a revolutionary effects system that provides fine-grained control over side effects, ensuring security and auditability.

## Core Philosophy

Manaknight is built on three fundamental principles:

1. **Immutability by Default**: All data is immutable, preventing accidental state mutations
2. **Capability-Based Security**: Explicit effect declarations control what code can do
3. **Totality**: All functions must terminate and cover all possible inputs

## Language Features

### Functional Programming with Effects

```manaknight
// Pure function - no effects allowed
fn add(x: Int64, y: Int64) -> Int64 {
    x + y
}

// Effectful function - explicitly declares what it can do
effect time
fn currentTime() -> Int64 {
    time.now()
}

// API endpoint with HTTP effect
effect http
api get "/users/:id" (id: String) -> Result<User, String> {
    http.get("https://api.example.com/users/" + id)
}
```

### Algebraic Data Types

```manaknight
type User = {
    id: Int64,
    name: String,
    email: String
}

type Result<T, E> = Ok(value: T) | Err(error: E)
```

### Pattern Matching

```manaknight
fn processResult(result: Result<String, String>) -> String {
    match result {
        Ok(value) => "Success: " + value,
        Err(error) => "Error: " + error
    }
}
```

### Pipelines

```manaknight
fn processUser(id: String) -> String {
    id |> parseId |> fetchUser |> formatUser
}
```

## Architecture

Manaknight consists of **5 major phases** that transform source code into secure, executable bytecode:

### Phase 1: Compiler Frontend

#### 1.1 AST Definitions
- Complete Abstract Syntax Tree for all Manaknight constructs
- C structs representing programs, modules, functions, types, effects, etc.
- Immutable tree structure for safe manipulation

#### 1.2 Lexer
- Converts Manaknight source into token stream
- Handles keywords, operators, literals, and API paths
- Tracks line/column information for precise error reporting

#### 1.3 Parser
- Recursive descent parser enforcing EBNF grammar
- Builds AST from token stream
- Reports syntax errors (E1000-E1999 range)

#### 1.4 Formatter
- Canonical source code formatting
- Ensures deterministic, idempotent output
- Language server integration ready

#### 1.5 Error Catalog
- Centralized error definitions (E1000-E9999)
- Structured error reporting with codes, messages, and locations
- Consistent error handling across all phases

### Phase 2: Semantic Analysis

#### 2.0 Module Resolution
- Maps `import user.auth` to `./user/auth.mk`
- Handles circular dependency detection (E5004)
- File system resolution with proper error handling

#### 2.1 Symbol Table & Scope
- Tracks variable/function declarations
- Nested scoping with shadowing prevention (E2006)
- Import aliasing and symbol resolution

#### 2.2 Type Checker
- Static type verification
- Function argument/return type checking
- Generic type inference and validation
- Comparison operator restrictions
- **Totality enforcement** (E2005) - all paths must return

#### 2.3 Effect Analyzer
- Infers effects from function bodies
- Verifies `inferred_effects ⊆ declared_effects`
- Prevents pure functions from calling effectful ones (E3002)
- Enforces lambda purity (E3004)

#### 2.4 Exhaustiveness Checker
- Ensures `match` expressions cover all ADT cases (E4001)
- Detects duplicate patterns (E4003)
- Supports wildcard patterns for completeness

### Phase 3: Code Generation

#### 3.0 Intrinsic Mapping Strategy
- `Int64` → C int64_t with overflow checks
- `String` → UTF-8 byte arrays
- `ADT` → Tagged JavaScript objects

#### 3.1 IR Lowering
- Transforms Manaknight AST to safe JavaScript subset
- `match` expressions → `if-else` chains
- Pipelines → nested function calls
- ADT constructors → tagged objects

#### 3.2 Effect Injection
- Modifies generated JS to pass `__effects` objects
- `time.now()` → `__effects.time.now()`
- Ensures effectful functions receive capability objects

#### 3.3 JS Emitter
- Generates ES5/ES6 compatible JavaScript
- Includes `"use strict"` and version metadata
- Avoids forbidden constructs (eval, with, etc.)

#### 3.4 OpenAPI Generator
- Maps Manaknight APIs to OpenAPI 3.0 specifications
- Type-safe API documentation generation
- REST endpoint specification

### Phase 4: Runtime & Standard Library

#### 4.0 Standard Library (.mk files)
- Manaknight source definitions for core types
- `Option<T>`, `Result<T, E>`, `List<T>`, `Map<K, V>`
- Effect declarations for all system interactions

#### 4.1 Standard Library (JS Implementation)
- JavaScript implementations of Tier-0 functions
- Int64 with overflow protection
- Immutable collections (List, Map)
- JSON handling with type safety

#### 4.2 Host Runtime (C)
- Extended MicroQuickJS with Manaknight support
- JS context creation and bytecode loading
- Resource limits and security boundaries

#### 4.3 Effect Handlers (C)
- Native implementations of all effects
- `time.now()`, `random.int()`, `http.get()`, etc.
- Secure system call wrappers

### Phase 5: Tools & Integration

#### 5.1 Compiler CLI (`mkc`)
- Command-line interface: `mkc input.mk -o output.js`
- Type checking: `mkc -c input.mk`
- OpenAPI generation: `mkc -a api.json input.mk`
- Professional error reporting and help

#### 5.2 End-to-End Testing
- Test harness: Source → `mkc` → JS → `mqjs` → Output
- Verifies compilation pipeline integrity
- Resource limit and security testing

## Security Model

### Capability-Based Effects

Manaknight uses **effects** as capabilities that must be explicitly declared and passed:

```manaknight
effect http, log, time

fn apiHandler(request: Request) -> Response {
    // Can only use declared effects
    log.info("Request received")
    let timestamp = time.now()
    let user = http.get("/api/user")

    // Cannot access filesystem, random numbers, etc.
    Response.ok(user)
}
```

### Runtime Security

1. **Effect Manifest**: Bytecode includes metadata about required effects
2. **Capability Injection**: Only declared effects are available at runtime
3. **Resource Limits**: Memory, CPU, and recursion limits prevent DoS
4. **Sandboxing**: Code runs in isolated MicroQuickJS contexts

## Execution Model

### Compilation to JavaScript

Manaknight compiles to a **safe JavaScript subset** that runs on MicroQuickJS:

```manaknight
effect time
fn greet(name: String) -> String {
    let hour = time.now() / 3600000 % 24
    if hour < 12 { "Good morning, " + name }
    else { "Good afternoon, " + name }
}
```

Compiles to:

```javascript
"use strict";

function greet(__effects, name) {
    var hour = __effects.time.now() / 3600000 % 24;
    if (hour < 12) {
        return "Good morning, " + name;
    } else {
        return "Good afternoon, " + name;
    }
}
```

### Runtime Environment

1. **MicroQuickJS** loads the generated JavaScript
2. **Effect objects** are created with native implementations
3. **Resource limits** are enforced
4. **Security policies** prevent unauthorized operations

## Key Innovations

### 1. Effects as Capabilities
Unlike traditional I/O permissions, Manaknight effects are **first-class values** that can be passed, stored, and composed.

### 2. Totality Checking
The type system enforces that all functions terminate, preventing infinite loops and ensuring predictable execution.

### 3. Deterministic Compilation
Canonical formatting and pure functional semantics ensure reproducible builds.

### 4. Secure by Construction
The effects system makes it impossible to accidentally introduce side effects or security vulnerabilities.

## Use Cases

- **Auditable APIs**: Financial services, healthcare, government systems
- **Serverless Functions**: Secure, resource-limited execution environments
- **Embedded Systems**: Deterministic, memory-safe code generation
- **Smart Contracts**: Type-safe, effect-controlled blockchain logic
- **Configuration DSLs**: Safe, auditable configuration languages

## Architecture Benefits

- **Memory Safety**: No buffer overflows, use-after-free, or null pointer dereferences
- **Type Safety**: Compile-time verification prevents runtime type errors
- **Effect Safety**: Explicit control over side effects prevents security issues
- **Deterministic**: Same input always produces same output
- **Auditable**: Every operation is explicitly declared and trackable

Manaknight represents a new paradigm in programming language design, combining the elegance of functional programming with the security of capability-based systems.
