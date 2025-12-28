# Manaknight Language Specification

## 1. Introduction

### Overview
Manaknight is a deterministic, functional, capability-based web language with first-class APIs and zero runtime ambiguity. It is designed to be:
*   Safer than JavaScript
*   Simpler than Rust
*   More expressive than Go for APIs
*   More auditable than Node/Python

### Design Philosophy
The language is built on strict guarantees ("Tier-0 Semantics") that enforce safety and determinism at the language level, not just the implementation level.

### Explicit Non-Goals
The following are explicitly **not supported** in v1:
*   Classes and inheritance
*   Reflection or introspection
*   Metaprogramming or macros
*   Dynamic code loading
*   Concurrency primitives
*   Shared mutable state
*   Implicit coercions

### Comparison to Other Languages
| Language | What Manaknight Matches or Beats |
| :--- | :--- |
| **Go** | Simpler semantics, stronger determinism |
| **Rust** | Easier APIs, fewer footguns |
| **Java** | Smaller stdlib, much safer |
| **Node** | Orders of magnitude safer |
| **WASM** | Easier to program, similar isolation |

---

## 2. Syntax & Grammar

### Formal Grammar (EBNF – Locked)
This grammar enforces **explicit `{}`**, no ambiguity, and no implicit scope.

```ebnf
program        ::= { module | api | declaration }

module         ::= "module" IDENT "{" { declaration } "}"

declaration    ::= function
                 | type_decl
                 | effect_decl
                 | import_decl

function       ::= "function" IDENT "(" [ params ] ")" [ ":" type ]
                   [ "uses" "{" effects "}" ]
                   block

params         ::= param { "," param }
param          ::= IDENT ":" type

block          ::= "{" { statement } "}"

statement      ::= let_stmt
                 | expr
                 | if_stmt
                 | match_stmt
                 | return_expr

let_stmt       ::= "let" IDENT "=" expr

if_stmt        ::= "if" expr block [ "else" block ]

match_stmt     ::= "match" expr "{" { match_case } "}"
match_case     ::= pattern "->" expr

type_decl      ::= "type" IDENT "{" type_body "}"
type_body      ::= record_body | union_body
record_body    ::= field { field }
field          ::= IDENT ":" type
union_body     ::= "|" IDENT [ "(" params ")" ] { "|" IDENT [ "(" params ")" ] }

effect_decl    ::= "effect" IDENT

api            ::= "api" HTTP_METHOD PATH block

expr           ::= literal
                 | IDENT
                 | call
                 | lambda
                 | pipeline

pipeline       ::= expr "|>" expr

call           ::= IDENT "(" [ args ] ")"
args           ::= expr { "," expr }

lambda         ::= "fn" "(" [ params ] ")" "=>" expr

type           ::= IDENT | IDENT "<" type ">"
```

**Hard guarantees:**
*   No dangling blocks
*   No optional scoping
*   No implicit returns except final expression
*   Grammar is LL/LR friendly

### Reserved Words
The following identifiers are reserved and may not be redefined or shadowed:
```text
function, let, type, effect, module, api,
if, else, match, fn, uses,
true, false,
some, none, ok, err,
language
```
Future versions may add keywords but never remove existing ones.

### Canonical Formatting
*   The language defines a canonical formatter.
*   Formatting is not semantically significant.
*   Tooling may reject non-canonically formatted source.
This enables deterministic builds and stable diffs.

---

## 3. Types & Data

### Primitive Types
*   **Int**: Signed 64-bit integer (Int64). Fixed size. Overflow results in runtime error. No implicit coercion.
*   **Bool**: `true` | `false`. No truthiness/falsy coercion.
*   **String**: Immutable sequence of UTF-8 codepoints.

### Composite Types
#### Structs / Records
```lang
type User {
  id: Int
  name: String
  email: String
}
```

#### Algebraic Data Types (ADTs)
```lang
type Payment {
  | CreditCard(number: String)
  | Wire(reference: String)
}
```

### No `null`, No `undefined`
These literals **do not exist**. Instead use:
```lang
type Option<T> {
  | some(value: T)
  | none
}

type Result<T, E> {
  | ok(value: T)
  | err(error: E)
}
```

### Numeric Semantics (Locked)
*   **Int64 Only**: No floats in v1.
*   **Overflow**: Checked. Runtime error on overflow (or compile-time if provable). No wrapping.
*   **Division**: Truncates toward zero. Division by zero is a runtime error.

### String Semantics (Locked)
*   Strings are immutable UTF-8.
*   Concatenation only via `+`.
*   Equality/Ordering based on UTF-8 codepoints.
*   No locale-dependent behavior.

### Equality Semantics
Equality is structural, total, and deterministic.

| Type | Equality Rule |
| :--- | :--- |
| Int | Value equality |
| Bool | Value equality |
| String | UTF-8 codepoint sequence equality |
| ADT | Same constructor AND field-wise equality |
| List | Recursive structural equality |
| Map | Key/value structural equality (order-independent) |
| Option | Structural equality |
| Result | Structural equality |
| Function | ❌ Not comparable (compile-time error) |

There is **no reference equality** in the language.

### Ordering Semantics
Only `Int` and `String` are orderable.
*   **Int**: Numeric ordering.
*   **String**: Lexicographic ordering (UTF-8).
*   Using comparison operators on other types is a compile-time error.

### Hashing Semantics
*   Hashing is structural and deterministic.
*   `equals(a, b) => hash(a) == hash(b)`.
*   Independent of memory layout or allocation identity.
*   Must not depend on Map iteration order.

### Serialization Semantics
*   All serializable values have a canonical encoding.
*   Structurally equivalent values serialize identically.
*   Map serialization order is deterministic.

---

## 4. Control Flow & Execution Semantics

### Functions (Pure by Default)
```lang
function add(a: Int, b: Int): Int {
  a + b
}
```
*   Last expression is returned.
*   No `return` keyword needed.
*   No mutation allowed.

### Immutability
```lang
let x = 5
let y = x + 1
x = 6        // ❌ compile error
```

### Conditionals
```lang
if age >= 18 {
  allow()
} else {
  deny()
}
```
*   No dangling `else`.
*   **Exhaustiveness**: `if` without `else` is forbidden.
*   Both branches must return the same type.

### Pattern Matching
```lang
match payment {
  CreditCard(n) -> processCard(n)
  Wire(ref)     -> processWire(ref)
}
```
*   Exhaustive checking required.
*   Compiler fails if a case is missing.

### Recursion & Tail Calls
*   **No TCO Guarantee**: The language does not guarantee tail-call optimization.
*   **Limits**: Recursion depth is limited at runtime. Exceeding it causes a controlled error.
*   Iteration should use folds/map.

### Memory Model
*   All values are deeply immutable.
*   Cycles are unrepresentable.
*   Structural sharing is permitted but not observable.

### Evaluation Order
*   Function arguments: Left-to-right.
*   Pipeline stages: Left-to-right.
*   Match scrutinee: Evaluated once.
*   Match cases: Tested top-to-bottom.
*   Map iteration: Undefined but deterministic for a given value.

### Variable Shadowing
*   **Forbidden**: Redeclaring an identifier in the same or nested scope is a compile-time error.

### Totality
*   All functions must return a value on all control paths.
*   Partial functions are not allowed.

### Termination Guarantees
*   Language does not guarantee termination (halting problem).
*   **But**: Infinite loops are impossible by syntax (no `while`/`for`).
*   Infinite recursion is halted by runtime limits.
*   Execution is bounded by resource budgets.

### Resource Accounting
Each execution is subject to:
*   Instruction budget
*   Memory budget
*   Allocation budget
Exhaustion results in a controlled runtime error (no crash).

---

## 5. Module System

### Module Declaration
```lang
module auth.user {
  export login
  export logout

  function login(...) { ... }
}
```
*   No global leakage.
*   Explicit exports only.

### Import Semantics
*   Imports are static and resolved at compile time.
*   No dynamic imports.
*   Paths are absolute module names.
```lang
import auth.user as userAuth
```

### Module Initialization
*   Modules contain declarations only.
*   No executable expressions or effects at module scope.
*   Modules are order-independent.

### Versioning
```lang
language v1.0
```
*   Source files must declare language version.
*   Compiler rejects incompatible versions.

### Dependencies (v1)
*   No external package manager.
*   All modules compiled together.
*   No network/dynamic dependency loading.

---

## 6. Effect System

### Philosophy
Effects represent **authority/capabilities**, not just behavior.
```lang
effect http
effect db
effect log
effect random
```

### Declaring Effects
```lang
function fetchUser(id: Int): Result<User, Error> uses { db } {
  db.findUser(id)
}
```
*   Pure functions **cannot** call effectful functions.
*   Functions must explicitly list **all** effects they use.
*   Effects do not propagate implicitly.

### Effect Composition
```lang
function handler(id: Int) uses { http, db } {
  let user = fetchUser(id) // uses { db }
  http.respond(json(user)) // uses { http }
}
```
Compiler verifies `inferred_effects ⊆ declared_effects`.

### Effect Inference Rules
1.  Literal/Identifier: ∅
2.  CallExpr: effects(callee)
3.  LambdaExpr: ∅ (must be pure)
4.  If/Match/Pipe: union(branches/stages)

### Forbidden
*   Hidden IO
*   Global state
*   Time/random without effect
*   Side effects in lambdas

### Time & Randomness
*   Access requires `effect time` / `effect random`.
*   No implicit entropy or clocks.
*   Enables deterministic testing.

---

## 7. API System (First-Class Feature)

### API Routes
APIs are language constructs.
```lang
api GET /users/:id {
  uses { db }
  let user = db.findUser(id)
  respond json(user)
}
```

### HTTP Types (Built-in)
*   **Request**: `method`, `path`, `headers` (Map), `body` (Json)
*   **Response**: `status`, `body` (Json)

### Automatic Features
*   Route registration
*   Param parsing & type validation
*   Effect scoping
*   JSON serialization
*   **OpenAPI Generation** (at compile time)

### API Execution Semantics
*   Effects executed strictly in source order.
*   No implicit retries.
*   No automatic rollback (idempotency is user responsibility).
*   Isolated VM context per request.

---

## 8. Compiler Architecture

### Pipeline
1.  **Lexer/Parser** (EBNF) -> AST
2.  **Type Checker**
3.  **Effect Analyzer**
4.  **Exhaustiveness Checker**
5.  **IR Lowering** (Functional Core)
6.  **JS Subset Lowering**
7.  **Bytecode Generation**
8.  **Sandboxed Execution**

### AST Design (Canonical)
The AST is the single source of truth.

**Top-Level AST**
```ts
Program {
  modules: Module[]
  apis: ApiRoute[]
}
```

**Modules**
```ts
Module {
  name: QualifiedName
  declarations: Declaration[]
}

type Declaration =
  | FunctionDecl
  | TypeDecl
  | EffectDecl
  | ImportDecl
```

**Functions**
```ts
FunctionDecl {
  name: Identifier
  params: Param[]
  returnType: Type
  effects: Effect[]        // empty = pure
  body: Block
}

Param {
  name: Identifier
  type: Type
}
```

**Blocks & Statements**
```ts
Block {
  statements: Statement[]
}

type Statement =
  | LetStmt
  | ExprStmt
  | IfStmt
  | MatchStmt

LetStmt {
  name: Identifier
  expr: Expr
}
```

**Expressions**
```ts
type Expr =
  | Literal
  | IdentifierExpr
  | CallExpr
  | LambdaExpr
  | IfExpr
  | MatchExpr
  | PipeExpr

CallExpr {
  callee: Identifier
  args: Expr[]
}

LambdaExpr {
  params: Param[]
  body: Expr
}
```

**Pattern Matching**
```ts
MatchExpr {
  expr: Expr
  cases: MatchCase[]
}

MatchCase {
  pattern: Pattern
  body: Expr
}

type Pattern =
  | ConstructorPattern
  | WildcardPattern
```

**Types**
```ts
type Type =
  | PrimitiveType
  | NamedType
  | GenericType
  | FunctionType

FunctionType {
  params: Type[]
  returnType: Type
  effects: Effect[]
}
```

### Type Checker Rules
*   **Let**: Infers type, binds immutably.
*   **Call**: Arg count/types match.
*   **If**: Condition is Bool, branches unify.
*   **Match**: Target is ADT, exhaustive, branches unify.

### Lowering to Safe JavaScript Subset
The language compiles to a **restricted JS IR**.
**Allowed**: `function`, `const`, `if/else`, object/array literals, explicit `return`.
**Forbidden**: `eval`, `this`, `class`, `try/catch`, `throw`, `null/undefined`, implicit globals, loops (use recursion/map).

#### Lowering Examples

**1. Functions**
```lang
function add(a: Int, b: Int): Int {
  a + b
}
```
↓
```js
function add(a, b) {
  return a + b;
}
```

**2. Let Bindings**
```lang
let x = expr
```
↓
```js
const x = <expr>;
```

**3. If Expressions**
```lang
if cond { a } else { b }
```
↓
```js
if (cond) {
  return a;
} else {
  return b;
}
```
(If used as expression, compiler wraps in IIFE-style block or hoists into temp variable).

**4. Pattern Matching (ADT)**
```lang
match x {
  some(v) -> v
  none    -> 0
}
```
↓
```js
if (x.tag === "some") {
  return x.value;
} else {
  return 0;
}
```

**5. Effects (Lowered as Parameters)**
```lang
function fetchUser(id: Int) uses { db } {
  db.findUser(id)
}
```
↓
```js
function fetchUser(id, __effects) {
  return __effects.db.findUser(id);
}
```

### Bytecode Generation
*   Compiles JS IR to deterministic bytecode (e.g., via MicroQuickJS).
*   Guarantees: Faster startup, no source exposure, ROM deployment.
*   **Reproducibility**: Identical source + compiler = identical bytecode.

### Compile-Time Forbidden Rules (Hard Errors)
*   Reassignment, Mutable data structures.
*   Unhandled patterns, Missing returns.
*   Undeclared effects, Recursive effects without base case.
*   Dynamic imports, Reflection.

---

## 9. Runtime Host

### Architecture
1.  **HTTP Server** -> Router
2.  **Request Validator**
3.  **VM Context** (Isolated)
4.  **Injected Effects**
5.  **Bytecode Execution**
6.  **Response Serializer**

### Isolation Model
*   Each request runs in its own VM context.
*   Fixed memory buffer.
*   No shared heap / global mutation.
*   Context destroyed on exit.

### Effect Injection
Effects are host objects injected per request.
```js
__effects = {
  db: { findUser: ... },
  http: { respond: ... }
}
```
User code cannot modify, store, or introspect these objects.

### Failure Handling
*   **Type/Effect Errors**: Compile-time.
*   **Logic Errors**: Return `Result.err`.
*   **Resource Limits (Timeout/OOM)**: Kill VM (Controlled error).
*   **Numeric Errors**: Controlled runtime error (no crash).
*   **Internal Errors**: Logged internally, generic error to user.

### Foreign Function Boundary
*   No FFI in v1.
*   All host interaction is via declared effects.

---

## 10. Error System Design

### Universal Error Object
```json
{
  "code": "E1XXX",
  "category": "SyntaxError | TypeError | ...",
  "message": "Human-readable explanation",
  "location": { "file": "...", "line": 1, "column": 1 }
}
```

### 1. Syntax & Parsing Errors (E1000–E1999)
*   `E1001` — Unexpected Token
*   `E1002` — Missing Closing Brace
*   `E1003` — Invalid Function Declaration
*   `E1004` — Invalid API Declaration
*   `E1005` — Invalid Type Declaration
*   `E1006` — Empty Block Not Allowed

### 2. Type System Errors (E2000–E2999)
*   `E2001` — Unknown Identifier
*   `E2002` — Type Mismatch
*   `E2003` — Invalid Function Call (Arg count mismatch)
*   `E2004` — Invalid Return Type
*   `E2005` — Missing Return Value
*   `E2006` — Reassignment Forbidden
*   `E2007` — Invalid Condition Type (Must be Bool)

### 3. Effect System Errors (E3000–E3999)
*   `E3001` — Undeclared Effect Usage
*   `E3002` — Effect Leakage (Pure function calling effectful one)
*   `E3003` — Effect Escalation
*   `E3004` — Effect Usage in Lambda
*   `E3005` — Invalid Effect Declaration

### 4. Pattern Matching Errors (E4000–E4999)
*   `E4001` — Non-Exhaustive Match
*   `E4002` — Invalid Match Target (Must be ADT)
*   `E4003` — Duplicate Pattern
*   `E4004` — Inconsistent Match Result Types

### 5. Module & Import Errors (E5000–E5999)
*   `E5001` — Module Not Found
*   `E5002` — Duplicate Module Definition
*   `E5003` — Symbol Not Exported
*   `E5004` — Circular Dependency

### 6. API Definition Errors (E6000–E6999)
*   `E6001` — Invalid HTTP Method
*   `E6002` — Invalid Route Path
*   `E6003` — Missing API Response
*   `E6004` — Invalid API Parameter Type
*   `E6005` — Undeclared Effect in API

### 7. Runtime Errors (E7000–E7999)
*   `E7001` — Invalid Result Value
*   `E7002` — Invalid Option Value
*   `E7003` — Serialization Failure
*   `E7004` — Invalid Bytecode

### 8. Resource Limit Errors (E8000–E8999)
*   `E8001` — Execution Timeout
*   `E8002` — Memory Limit Exceeded
*   `E8003` — Recursion Limit Exceeded
*   `E8004` — Allocation Limit Exceeded

### 9. Internal Errors (E9000–E9999)
*   `E9001` — Compiler Internal Error
*   `E9002` — VM Internal Error

### Public API Errors
*   No stack traces.
*   No internal filenames/addresses.
*   Returns standard error JSON with safe code/message.

---

## 11. Standard Library

### Core Principles
*   No mutation, global state, or side effects.
*   All values serializable.

### Tier-0: Mandatory Libraries (v1)

#### 1. Core / Prelude (Always Imported)
**Types**: `Int`, `Bool`, `String`, `Option<T>`, `Result<T,E>`, `List<T>`, `Map<K,V>`, `Json`.
**Functions**:
*   `identity(x)`
*   `equals(a, b)`
*   `hash(x)`
*   `pipe(x, f)`
*   `compose(f, g)`
*   `not(bool)`, `and(a, b)`, `or(a, b)`
**Option**: `map`, `flatMap`, `unwrapOr`, `isSome`, `isNone`
**Result**: `map`, `flatMap`, `mapError`, `unwrapOr`

#### 2. Math (Pure, Int-Only)
**Functions**:
*   `add(a, b)`
*   `sub(a, b)`
*   `mul(a, b)`
*   `div(a, b)` (Truncates toward zero)
*   `mod(a, b)`
*   `abs(n)`
*   `min(a, b)`
*   `max(a, b)`
*   `clamp(n, min, max)`
**Rules**: Int64 only. Overflow = runtime error. No floats.

#### 3. String (Pure, Deterministic)
**Functions**:
*   `length(s)`
*   `isEmpty(s)`
*   `concat(a, b)`
*   `split(s, sep)`
*   `join(list, sep)`
*   `contains(s, sub)`
*   `startsWith(s, prefix)`
*   `endsWith(s, suffix)`
*   `toUpperAscii(s)`
*   `toLowerAscii(s)`
**Rules**: UTF-8. No locale magic.

#### 4. List (Functional Backbone)
**Functions**:
*   `map(list, fn)`
*   `flatMap(list, fn)`
*   `filter(list, fn)`
*   `fold(list, init, fn)`
*   `foldRight(list, init, fn)`
*   `length(list)`
*   `reverse(list)`
*   `take(list, n)`
*   `drop(list, n)`
*   `find(list, fn)`
*   `all(list, fn)`
*   `any(list, fn)`

#### 5. Map (Key-Value Backbone)
**Functions**:
*   `empty()`
*   `get(map, key)`
*   `set(map, key, value)`
*   `remove(map, key)`
*   `containsKey(map, key)`
*   `keys(map)`
*   `values(map)`
*   `merge(m1, m2)`
**Rules**: Structural keys only. Deterministic hashing. Order-independent equality.

#### 6. Json (Boundary-Only)
**Functions**:
*   `encode(val)`
*   `decode(string)`
*   `get(json, key)`
*   `getString(json)`
*   `getInt(json)`
*   `getObject(json)`
*   `getArray(json)`

#### 7. HTTP (Effectful)
**Effect**: `http`
**Types**: `Request`, `Response`
**Functions**:
*   `getHeader(req, name)`
*   `setHeader(res, name, value)`
*   `json(body)`
*   `text(body)`
*   `status(code)`

#### 8. Log (Effectful)
**Effect**: `log`
**Functions**:
*   `info(msg, data)`
*   `warn(msg, data)`
*   `error(msg, data)`

#### 9. Time (Effectful)
**Effect**: `time`
**Functions**:
*   `now()`
*   `unixMillis()`

#### 10. Random (Effectful)
**Effect**: `random`
**Functions**:
*   `int(min, max)`
*   `bytes(length)`

### Tier-1: Strongly Recommended
*   **Validate**: `nonEmpty`, `minLength`, `maxLength`, `matches`, `inRange`.
*   **Error**: `new`, `withCode`, `withMessage`.
*   **Encoding**: `base64`, `hex` (`encode`, `decode`).
*   **Crypto (Effectful)**: `hashSha256`, `hmacSha256` (Safe defaults only).
*   **ID (Effectful)**: `uuidV4`.

### Tier-2: Optional
*   **Storage (Effectful)**: `read`, `write`, `exists` (Sandboxed).
*   **Test Helpers**: `assert`, `assertEquals`, `assertError` (Dev-only).

### MUST NOT be in Stdlib
*   Threading/Concurrency
*   Global mutable state
*   Reflection
*   ORM/DB abstractions
*   OS access / Env vars
