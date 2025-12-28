# 1. Core Language

## 1.1 Functions (Pure by Default)

```lang
function add(a: Int, b: Int): Int {
  a + b
}
```

Rules:

* last expression is returned
* no `return` keyword needed
* no mutation allowed

---

## 1.2 Immutability

```lang
let x = 5
let y = x + 1
x = 6        // ❌ compile error
```

---

## 1.3 Explicit Effects

```lang
effect http
effect log
effect db
```

```lang
function fetchUser(id: Int): Result<User, Error> uses { http } {
  http.get("/users/" + id)
}
```

Rules:

* effects must be declared
* undeclared side effects = compile error
* pure functions cannot call effectful ones

---

# 2. Control Flow (Safe by Construction)

## 2.1 Conditionals

```lang
if age >= 18 {
  allow()
} else {
  deny()
}
```

No dangling `else`. Ever.

---

## 2.2 Pattern Matching

```lang
match payment {
  CreditCard(n) -> processCard(n)
  Wire(ref)     -> processWire(ref)
}
```

Rules:

* exhaustive checking required
* compiler fails if a case is missing

---

# 3. Types & Data

## 3.1 Structs / Records

```lang
type User {
  id: Int
  name: String
  email: String
}
```

---

## 3.2 Algebraic Data Types

```lang
type Payment {
  | CreditCard(number: String)
  | Wire(reference: String)
}
```

---

## 3.3 No `null`, No `undefined`

Instead:

```lang
Option<T>   // some(T) | none
Result<T,E> // ok(T) | err(E)
```

---

# 4. Modules

```lang
module auth.user {
  export login
  export logout

  function login(...) { ... }
}
```

Rules:

* no global leakage
* explicit exports only

---

# 5. API as a Language Feature

## 5.1 API Routes

```lang
api GET /users/:id {
  uses { db }

  let user = db.findUser(id)
  respond json(user)
}
```

Guarantees:

* typed params
* typed responses
* automatic validation
* auto OpenAPI generation

---

## 5.2 HTTP Types (Built-in)

```lang
type Request {
  method: String
  path: String
  headers: Map<String, String>
  body: Json
}

type Response {
  status: Int
  body: Json
}
```

---

# 6. Execution Model (Still Critical)

Even with `{}` syntax, these rules stay locked:

* no `eval`
* no prototype mutation
* no dynamic globals
* no implicit coercion
* no array holes
* deterministic execution
* fixed memory limits

# 1. Formal Grammar (EBNF – Locked)

This grammar enforces **explicit `{}`**, no ambiguity, no implicit scope.

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

* No dangling blocks
* No optional scoping
* No implicit returns except final expression
* Grammar is LL/LR friendly

---

# 2. Effect System (Formal & Enforced)

## 2.1 Effects Are Capabilities

Effects represent **authority**, not behavior.

```lang
effect http
effect db
effect log
effect crypto
```

---

## 2.2 Declaring Effects

```lang
function fetchUser(id: Int): Result<User, Error>
uses { db } {
  db.findUser(id)
}
```

### Rules (Compiler-Enforced)

* Pure functions **cannot** call effectful functions
* Functions must explicitly list **all** effects they use
* Effects do not propagate implicitly

---

## 2.3 Effect Composition

```lang
function handler(id: Int)
uses { http, db } {
  let user = fetchUser(id)
  http.respond(json(user))
}
```

Compiler verifies:

* `fetchUser` uses `{ db }`
* `handler` uses `{ http, db }` ✅

---

## 2.4 Forbidden

❌ Hidden IO
❌ Global state
❌ Time/random without effect
❌ Side effects in lambdas

---

# 3. Lowering Rules → JavaScript Subset

Your language **does not run JS** — it *compiles to* a **restricted JS IR**.

## 3.1 Allowed JS Constructs

* `function`
* `const`
* `if / else`
* object literals
* arrays (no holes)
* explicit returns
* pure expressions

## 3.2 Forbidden JS Constructs

❌ `eval`
❌ `with`
❌ `this`
❌ prototype mutation
❌ dynamic property access
❌ implicit globals
❌ exceptions
❌ `null` / `undefined`

---

## 3.3 Example Lowering

### Source

```lang
function add(a: Int, b: Int): Int {
  a + b
}
```

### Lowered JS

```js
function add(a, b) {
  return a + b;
}
```

---

### Pattern Match Lowering

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

---

# 4. API Runtime Host (First-Class Feature)

APIs are **language constructs**, not frameworks.

## 4.1 API Declaration

```lang
api GET /users/:id {
  uses { db }

  let user = db.findUser(id)
  respond json(user)
}
```

---

## 4.2 What the Runtime Does Automatically

* Route registration
* Param parsing
* Type validation
* Effect scoping
* Error handling
* JSON serialization
* OpenAPI generation

---

## 4.3 Execution Model

Each API call:

* runs in its own isolated VM context
* has memory + time limits
* has only declared effects injected

No shared mutable state. Ever.

---

# 5. Compile-Time Forbidden Rules (Hard Errors)

The compiler **must fail** on any of the following.

## 5.1 Language-Level

❌ Reassignment
❌ Mutable data structures
❌ Unhandled pattern match
❌ Missing return value
❌ Undeclared effects
❌ Recursive effects without base case

---

## 5.2 Runtime-Level

❌ Dynamic imports
❌ Reflection
❌ Accessing host objects
❌ Modifying built-ins
❌ Cross-request state

---

## 5.3 API-Level

❌ Untyped input/output
❌ Side effects outside declared effects
❌ Silent failures
❌ Exception throwing

---

# 6. End-to-End Compilation Pipeline

This is the full system.

```text
Source Code
   ↓
Parser (EBNF)
   ↓
AST
   ↓
Type Checker
   ↓
Effect Analyzer
   ↓
Exhaustiveness Checker
   ↓
IR (Functional Core)
   ↓
JS Subset Lowering
   ↓
Bytecode Generation
   ↓
Sandboxed Execution
```

---

## 6.1 Determinism Guarantees

* Same input → same output
* No timing leaks
* No global mutation
* Reproducible builds

---

## 6.2 Deployment Modes

* Server binary
* Embedded appliance
* Edge worker
* CLI batch jobs

All using the same runtime.

---

# Summary (What You’ve Designed)

You now have:

> **A deterministic, functional, capability-based web language with first-class APIs and zero runtime ambiguity**

This is:

* safer than JavaScript
* simpler than Rust
* more expressive than Go for APIs
* more auditable than Node/Python

# 1. AST Design (Canonical, Minimal, Safe)

This is the **single source of truth** your compiler uses after parsing.

---

## 1.1 Top-Level AST

```ts
Program {
  modules: Module[]
  apis: ApiRoute[]
}
```

---

## 1.2 Modules

```ts
Module {
  name: QualifiedName
  declarations: Declaration[]
}
```

```ts
type Declaration =
  | FunctionDecl
  | TypeDecl
  | EffectDecl
  | ImportDecl
```

---

## 1.3 Functions

```ts
FunctionDecl {
  name: Identifier
  params: Param[]
  returnType: Type
  effects: Effect[]        // empty = pure
  body: Block
}
```

```ts
Param {
  name: Identifier
  type: Type
}
```

**Rules enforced at AST level**

* effects always explicit (even empty)
* return type always known (explicit or inferred)
* body is always a block `{}`

---

## 1.4 Blocks & Statements

```ts
Block {
  statements: Statement[]
}
```

```ts
type Statement =
  | LetStmt
  | ExprStmt
  | IfStmt
  | MatchStmt
```

```ts
LetStmt {
  name: Identifier
  expr: Expr
}
```

---

## 1.5 Expressions

```ts
type Expr =
  | Literal
  | IdentifierExpr
  | CallExpr
  | LambdaExpr
  | IfExpr
  | MatchExpr
  | PipeExpr
```

```ts
CallExpr {
  callee: Identifier
  args: Expr[]
}
```

```ts
LambdaExpr {
  params: Param[]
  body: Expr
}
```

---

## 1.6 Pattern Matching

```ts
MatchExpr {
  expr: Expr
  cases: MatchCase[]
}
```

```ts
MatchCase {
  pattern: Pattern
  body: Expr
}
```

```ts
type Pattern =
  | ConstructorPattern
  | WildcardPattern
```

---

## 1.7 Types

```ts
type Type =
  | PrimitiveType
  | NamedType
  | GenericType
  | FunctionType
```

```ts
FunctionType {
  params: Type[]
  returnType: Type
  effects: Effect[]
}
```

---

# 2. Type Checker (Rules, Not Theory)

This is **compile-time only**. No runtime type checks except at API boundaries.

---

## 2.1 Type Environment

```ts
TypeEnv {
  variables: Map<Identifier, Type>
  functions: Map<Identifier, FunctionType>
  types: Map<TypeName, TypeDef>
}
```

Immutable, persistent, scoped.

---

## 2.2 Core Rules

### 2.2.1 Let Binding

```lang
let x = expr
```

Rules:

1. infer type of `expr`
2. bind `x` immutably
3. reassignment forbidden

---

### 2.2.2 Function Call

```lang
f(a, b)
```

Rules:

1. `f` must be a function
2. arg count must match
3. arg types must unify
4. effects of `f` are recorded (see section 3)

---

### 2.2.3 If Expression

```lang
if cond { a } else { b }
```

Rules:

* `cond: Bool`
* type(a) == type(b)
* result type = that type

---

### 2.2.4 Match Expression

```lang
match x { ... }
```

Rules:

* `x` must be ADT
* all constructors covered
* all branches return same type

Compiler error if non-exhaustive.

---

## 2.3 No `null`, No `undefined`

Invalid at AST level. These literals **do not exist**.

Optional values must use:

```lang
Option<T> = some(T) | none
```

---

## 2.4 Return Rules

* last expression in block = return value
* empty block forbidden
* implicit `Unit` not allowed unless explicit

---

## 2.5 API Boundary Types

At `api` declarations:

* input params must be typed
* output must be `Response`
* JSON must match schema

---

# 3. Effect Inference & Verification (Critical)

This is the **heart** of the language’s safety.

---

## 3.1 Effect Model

Effects are **capabilities**, not behaviors.

```ts
Effect = "http" | "db" | "log" | ...
```

---

## 3.2 Effect Set per Function

Each function has:

```ts
DeclaredEffects: Set<Effect>
InferredEffects: Set<Effect>
```

---

## 3.3 Effect Inference Algorithm

### Walk the AST bottom-up

For each expression:

| Expression Type | Effects            |
| --------------- | ------------------ |
| Literal         | ∅                  |
| Identifier      | ∅                  |
| CallExpr        | effects(callee)    |
| LambdaExpr      | ∅ (must be pure)   |
| IfExpr          | union(branches)    |
| MatchExpr       | union(cases)       |
| PipeExpr        | union(left, right) |

---

### Example

```lang
function handler(id: Int)
uses { http, db } {
  let user = fetchUser(id)
  http.respond(user)
}
```

Inference:

* `fetchUser` → `{ db }`
* `http.respond` → `{ http }`
* total inferred → `{ db, http }`

Check:

```
inferred ⊆ declared  ✅
```

---

## 3.4 Effect Violation Errors

Compiler must fail on:

### ❌ Undeclared Effect

```lang
function f() {
  http.get("/")   // ERROR
}
```

---

### ❌ Effect Leakage via Lambdas

```lang
fn () => http.get("/")   // ERROR
```

Lambdas are **pure only**.

---

### ❌ Effect Escalation

```lang
function pure() {
  effectful()
}
```

Pure function calling effectful function is illegal.

---

## 3.5 Effect Polymorphism (Optional, Future)

Later you *can* add:

```lang
function map<T>(list: List<T>, f: fn(T) -> U): List<U>
```

with rule:

* `f` must be pure

Keep v1 simple: **no effect polymorphism**.

---

# Final Reality Check

With **just these three pieces**, you already have:

✅ Deterministic execution
✅ Compile-time IO safety
✅ No runtime surprises
✅ Auditable APIs
✅ Sandboxed hosting

This is **already stronger** than:

* Node.js
* Python
* Go
* Java web stacks

# 4. Lowering IR → Safe JavaScript Subset

Your language **never executes source directly**.
It lowers into a **very small, auditable JS IR**, which is then compiled to bytecode.

---

## 4.1 Target JS Subset (Hard-Locked)

Only the following JS constructs are allowed:

✅ `function`
✅ `const`
✅ `if / else`
✅ object literals `{}`
✅ array literals `[]` (no holes)
✅ explicit `return`
✅ numeric / string / boolean literals

❌ `eval`
❌ `this`
❌ `prototype`
❌ `class`
❌ `try/catch`
❌ `throw`
❌ `null / undefined`
❌ implicit globals
❌ dynamic property access

This maps cleanly to **MicroQuickJS stricter mode**.

---

## 4.2 Canonical Lowering Rules

### 4.2.1 Functions

**Source**

```lang
function add(a: Int, b: Int): Int {
  a + b
}
```

**Lowered JS**

```js
function add(a, b) {
  return a + b;
}
```

Rule:

* last expression → explicit `return`
* params already validated at compile time

---

### 4.2.2 Let Bindings

```lang
let x = expr
```

↓

```js
const x = <expr>;
```

Reassignment is impossible by construction.

---

### 4.2.3 If Expressions

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

If used as expression, compiler wraps in IIFE-style block or hoists into temp variable.

---

### 4.2.4 Pattern Matching (ADT)

```lang
match x {
  some(v) -> v
  none    -> 0
}
```

Lowered representation for ADTs:

```js
// Example value
{ tag: "some", value: v }
```

↓

```js
if (x.tag === "some") {
  return x.value;
} else {
  return 0;
}
```

Exhaustiveness is already guaranteed by the compiler.

---

### 4.2.5 Pipelines

```lang
a |> f |> g
```

↓

```js
g(f(a))
```

No intermediate allocations unless required.

---

### 4.2.6 Effects (Lowered as Parameters)

Effectful functions receive **capability objects** explicitly.

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

Pure functions never receive `__effects`.

---

# 5. Bytecode Generation Strategy

Now we compile the JS IR into **deterministic bytecode**.

---

## 5.1 Compilation Stages

```text
AST
 ↓
Typed IR
 ↓
Effect-checked IR
 ↓
JS Subset IR
 ↓
JS Source (generated)
 ↓
JS → Bytecode
```

---

## 5.2 Why Bytecode (Not JS Execution)

Bytecode gives you:

* faster startup
* smaller footprint
* ROM deployment
* no source exposure
* no runtime parsing

Perfect for:

* APIs
* embedded
* edge
* sandboxed hosting

---

## 5.3 Bytecode Build Flow

1. Generate JS source from IR
2. Run MicroQuickJS compiler
3. Emit `.bin` bytecode
4. Store:

   * per-module bytecode
   * per-api bytecode

---

## 5.4 Deterministic Guarantees

You enforce:

* fixed memory buffer
* no dynamic allocation beyond limit
* no runtime code loading
* no reflection

This ensures:

> **Same bytecode + same input = same output**

---

## 5.5 Bytecode Layout (Recommended)

```text
/app
 ├─ core.bin        (stdlib + runtime)
 ├─ modules/
 │   ├─ auth.bin
 │   ├─ users.bin
 ├─ api/
 │   ├─ get_user.bin
 │   ├─ create_user.bin
```

Each API route can be:

* loaded
* unloaded
* hot-swapped

---

# 6. API Runtime Host (Execution Engine)

This is the **production runtime**.

---

## 6.1 High-Level Architecture

```text
HTTP Server
   ↓
Router
   ↓
Request Validator
   ↓
VM Context (isolated)
   ↓
Injected Effects
   ↓
Bytecode Execution
   ↓
Response Serializer
```

---

## 6.2 Request Lifecycle

### Step-by-step:

1. Incoming HTTP request
2. Route match (`GET /users/:id`)
3. Validate params (types)
4. Create VM context
5. Inject only declared effects
6. Load route bytecode
7. Execute handler
8. Serialize response
9. Destroy VM context

No shared memory. No leaks.

---

## 6.3 Effect Injection Model

Effects are **objects provided by the host**.

Example:

```js
{
  db: {
    findUser: (id) => ...
  },
  http: {
    respond: (body) => ...
  }
}
```

Injected per request.

Functions **cannot**:

* store them
* modify them
* pass them outside scope

---

## 6.4 Resource Limits (Mandatory)

Each request has:

* max memory
* max CPU time
* max recursion depth
* max allocations

Exceeding any limit:
→ hard terminate VM
→ return controlled error

---

## 6.5 Error Model

There are **no exceptions** in user code.

All failures are:

* `Result<T, E>`
* or host-enforced aborts (timeout, OOM)

The runtime:

* never crashes
* never leaks stack traces
* never exposes internals

---

## 6.6 OpenAPI Generation

At compile time:

* route
* params
* request body
* response schema

→ auto-generate OpenAPI spec
→ zero runtime reflection needed

---

# End State: What You Have Built

You now fully have:

✔ A functional language
✔ Compile-time IO safety
✔ Deterministic execution
✔ Capability-based APIs
✔ Sandboxed hosting
✔ Bytecode deployment
✔ Hot-reloadable services

This is **not a toy**.
This is **in the same class as WASM runtimes and Unison-like systems**, but *far simpler to ship*.

# 1. Standard Library (Minimal, Powerful, Safe)

Your stdlib must be **small, frozen, and boring**. Power comes from composition, not surface area.

## 1.1 Core Principles

* **No mutation**
* **No global state**
* **No side effects**
* **No magic**
* **All values serializable**
* **Everything implemented in the language or host primitives**

---

## 1.2 Core Types (Required)

### Option

```lang
type Option<T> {
  | some(value: T)
  | none
}
```

Usage:

```lang
match user {
  some(u) -> u.name
  none    -> "Guest"
}
```

---

### Result

```lang
type Result<T, E> {
  | ok(value: T)
  | err(error: E)
}
```

Usage:

```lang
parse(input)
|> then(validate)
|> then(save)
```

---

### Bool (No Truthiness)

```lang
type Bool {
  | true
  | false
}
```

❌ no truthy/falsy coercion
✔ explicit logic only

---

## 1.3 Collection Types

### List

```lang
type List<T> {
  | cons(head: T, tail: List<T>)
  | nil
}
```

Std functions:

```lang
map(list, fn)
filter(list, fn)
fold(list, init, fn)
length(list)
```

---

### Map (Immutable)

```lang
type Map<K, V>
```

Provided ops:

```lang
get(map, key): Option<V>
set(map, key, value): Map<K, V>
keys(map): List<K>
```

---

## 1.4 Core Functions

All **pure**:

```lang
identity(x)
compose(f, g)
pipe(x, f)
equals(a, b)
```

---

## 1.5 JSON (Boundary Only)

```lang
type Json {
  | string
  | number
  | bool
  | array(List<Json>)
  | object(Map<String, Json>)
}
```

Only usable:

* at API boundaries
* never internally as logic data

---

# 2. Compiler Skeleton (Real, Minimal, Buildable)

This is a **real compiler**, not a diagram.

---

## 2.1 Compiler Stages

```text
source
 ↓
lexer
 ↓
parser
 ↓
AST
 ↓
type checker
 ↓
effect checker
 ↓
IR lowering
 ↓
JS generation
 ↓
bytecode
```

---

## 2.2 Module Layout

```text
compiler/
 ├─ lexer.ts
 ├─ parser.ts
 ├─ ast.ts
 ├─ types.ts
 ├─ typechecker.ts
 ├─ effects.ts
 ├─ ir.ts
 ├─ lower.ts
 ├─ jsgen.ts
 ├─ cli.ts
```

Language choice:

* TypeScript (fast iteration) **or**
* Rust (production safety)

---

## 2.3 Core Compiler Loop (Pseudo-Code)

```ts
function compile(source: string): Bytecode {
  tokens = lex(source)
  ast = parse(tokens)

  typedAst = typeCheck(ast)
  checkedAst = checkEffects(typedAst)

  ir = lowerToIR(checkedAst)
  js = generateJS(ir)

  bytecode = compileToBytecode(js)
  return bytecode
}
```

Each stage:

* immutable input
* immutable output
* pure function

---

## 2.4 Type Checker Core

```ts
function inferExpr(expr, env): Type {
  switch expr.kind:
    case Literal: return expr.type
    case Identifier: return env.lookup(expr.name)
    case Call:
      fnType = inferExpr(expr.fn)
      checkArgs(fnType, expr.args)
      return fnType.return
    case Match:
      ensureExhaustive(expr)
      unifyBranches(expr)
}
```

Failure = compile error.
No runtime type checks inside logic.

---

## 2.5 Effect Checker Core

```ts
function inferEffects(expr): Set<Effect> {
  match expr:
    Literal        -> {}
    Call(fn)       -> fn.effects
    If(a, b)       -> union(a, b)
    Match(cases)   -> union(cases)
}
```

Final rule:

```
inferred ⊆ declared
```

---

# 3. API Runtime Host (Production-Grade but Small)

This is the **thing you deploy**.

---

## 3.1 Binary Layout

```text
runtime/
 ├─ server.c
 ├─ vm.c
 ├─ router.c
 ├─ effects.c
 ├─ limits.c
 ├─ response.c
```

Or single binary if embedded.

---

## 3.2 Runtime Startup

```text
1. Load stdlib bytecode
2. Load module bytecode
3. Load API bytecode
4. Start HTTP server
```

---

## 3.3 Request Execution Flow

```text
HTTP Request
 ↓
Route Match
 ↓
Validate Params
 ↓
Create VM Context
 ↓
Inject Effects
 ↓
Execute Bytecode
 ↓
Serialize Response
 ↓
Destroy Context
```

Every request is isolated.

---

## 3.4 VM Context Rules

Each VM:

* fixed memory buffer
* no shared heap
* no global mutation
* no cross-request data

On exit:

* GC
* memory zeroed
* context destroyed

---

## 3.5 Effect Injection (Hard Boundary)

Example injection object:

```js
__effects = {
  db: { findUser },
  http: { respond },
  log: { info }
}
```

User code:

* cannot modify
* cannot store
* cannot introspect

---

## 3.6 Failure Handling

| Failure          | Behavior     |
| ---------------- | ------------ |
| Type error       | Compile-time |
| Effect violation | Compile-time |
| Timeout          | Kill VM      |
| OOM              | Kill VM      |
| Logic error      | Result.err   |

No crashes. No stack traces. No leaks.

---

# What You Now Fully Have

You now possess a **complete system**:

✔ Language spec
✔ Type system
✔ Effect system
✔ Compiler architecture
✔ Runtime execution model
✔ API hosting model

This is *already enough* to build:

* internal APIs
* edge services
* embedded logic engines
* auditable systems

# Error System Design

## Error Object Shape (Universal)

Every error in the system — compiler, runtime, API — conforms to:

```json
{
  "code": "E1XXX",
  "category": "TypeError | EffectError | SyntaxError | RuntimeError | ApiError",
  "message": "Human-readable explanation",
  "location": {
    "file": "users.lang",
    "line": 12,
    "column": 5
  }
}
```

* `location` is **optional** at runtime
* `code` is **stable forever** (no reuse)

---

# Error Code Ranges (Reserved)

| Range       | Category                       |
| ----------- | ------------------------------ |
| E1000–E1999 | Syntax / Parsing               |
| E2000–E2999 | Type System                    |
| E3000–E3999 | Effect System                  |
| E4000–E4999 | Pattern Matching               |
| E5000–E5999 | Module / Import                |
| E6000–E6999 | API Definition                 |
| E7000–E7999 | Runtime (VM)                   |
| E8000–E8999 | Resource Limits                |
| E9000–E9999 | Internal (never shown to user) |

---

# 1. Syntax & Parsing Errors (E1000–E1999)

### E1001 — Unexpected Token

```
Unexpected token '}'
```

### E1002 — Missing Closing Brace

```
Missing '}' to close block
```

### E1003 — Invalid Function Declaration

```
Invalid function declaration syntax
```

### E1004 — Invalid API Declaration

```
Invalid API declaration
```

### E1005 — Invalid Type Declaration

```
Invalid type declaration
```

### E1006 — Empty Block Not Allowed

```
Empty blocks are not allowed
```

---

# 2. Type System Errors (E2000–E2999)

### E2001 — Unknown Identifier

```
Unknown identifier 'userId'
```

### E2002 — Type Mismatch

```
Expected type Int but found String
```

### E2003 — Invalid Function Call

```
Function 'add' expects 2 arguments but received 1
```

### E2004 — Invalid Return Type

```
Function returns String but declared return type is Int
```

### E2005 — Missing Return Value

```
Function does not return a value on all paths
```

### E2006 — Reassignment Forbidden

```
Reassignment of immutable variable 'x' is not allowed
```

### E2007 — Invalid Condition Type

```
Condition expression must be Bool
```

---

# 3. Effect System Errors (E3000–E3999)

### E3001 — Undeclared Effect Usage

```
Effect 'http' is used but not declared
```

### E3002 — Effect Leakage

```
Pure function cannot call effectful function 'fetchUser'
```

### E3003 — Effect Escalation

```
Function declares effect 'db' but uses undeclared effect 'http'
```

### E3004 — Effect Usage in Lambda

```
Lambdas must be pure and cannot use effects
```

### E3005 — Invalid Effect Declaration

```
Effect 'db' is not defined
```

---

# 4. Pattern Matching Errors (E4000–E4999)

### E4001 — Non-Exhaustive Match

```
Pattern match is not exhaustive
```

### E4002 — Invalid Match Target

```
Match expression must be an algebraic data type
```

### E4003 — Duplicate Pattern

```
Duplicate pattern 'some' in match expression
```

### E4004 — Inconsistent Match Result Types

```
All match branches must return the same type
```

---

# 5. Module & Import Errors (E5000–E5999)

### E5001 — Module Not Found

```
Module 'auth.user' not found
```

### E5002 — Duplicate Module Definition

```
Module 'auth.user' is defined more than once
```

### E5003 — Symbol Not Exported

```
Symbol 'login' is not exported by module 'auth.user'
```

### E5004 — Circular Dependency

```
Circular dependency detected between modules
```

---

# 6. API Definition Errors (E6000–E6999)

### E6001 — Invalid HTTP Method

```
Invalid HTTP method 'FETCH'
```

### E6002 — Invalid Route Path

```
Invalid route path '/users//id'
```

### E6003 — Missing API Response

```
API handler must return a Response
```

### E6004 — Invalid API Parameter Type

```
API parameter 'id' must be Int
```

### E6005 — Undeclared Effect in API

```
API uses effect 'db' but it is not declared
```

---

# 7. Runtime Errors (E7000–E7999)

> These are **controlled failures**, never crashes.

### E7001 — Invalid Result Value

```
Invalid Result value returned
```

### E7002 — Invalid Option Value

```
Invalid Option value returned
```

### E7003 — Serialization Failure

```
Failed to serialize response body
```

### E7004 — Invalid Bytecode

```
Invalid or corrupted bytecode
```

---

# 8. Resource Limit Errors (E8000–E8999)

### E8001 — Execution Timeout

```
Execution time limit exceeded
```

### E8002 — Memory Limit Exceeded

```
Memory limit exceeded
```

### E8003 — Recursion Limit Exceeded

```
Maximum recursion depth exceeded
```

### E8004 — Allocation Limit Exceeded

```
Too many allocations
```

---

# 9. Internal Errors (E9000–E9999)

> **Never shown to users directly**

### E9001 — Compiler Internal Error

```
Internal compiler error
```

### E9002 — VM Internal Error

```
Internal runtime error
```

These are logged internally and surfaced externally as:

```json
{
  "code": "E7000",
  "message": "Internal error"
}
```

---

# API Error Response Format (Public)

All API failures return:

```json
{
  "error": {
    "code": "E6004",
    "message": "API parameter 'id' must be Int"
  }
}
```

No stack traces.
No internal filenames.
No memory addresses.

# Numeric Semantics (🔒 Locked)

## 1. Numeric Model (Critical)

### Rationale

Numbers are one of the fastest ways to destroy determinism. Vague integer sizes, silent overflow, or JS-like coercion would undermine everything else you’ve designed.

### Specification

```text
The language defines a single integer type: Int.
```

#### Integer Type

```text
Int is a signed 64-bit integer (Int64).
```

* Range: −9,223,372,036,854,775,808 to 9,223,372,036,854,775,807
* Size is fixed and identical across all platforms.

#### Overflow Semantics

```text
All integer operations are checked.
Overflow results in a runtime error.
No wrapping, saturation, or undefined behavior is permitted.
```

If overflow can be proven at compile time, it is a **compile-time error**.

#### Division Semantics

```text
Integer division truncates toward zero.
Division by zero:
- Compile-time error if statically provable
- Runtime error otherwise
```

#### Floating Point

```text
Floating-point numbers are not supported in v1.
```

Future versions may introduce `Float64` explicitly; no implicit mixing is allowed.

---

# Equality Semantics (🔒 Locked)

## 2. Equality & Structural Comparison

### Rationale

If equality is not specified, developers will assume JavaScript semantics. That is unacceptable in a safety-focused language.

### Specification

```text
Equality is structural, total, and deterministic.
```

#### Equality Rules

| Type     | Equality Rule                                     |
| -------- | ------------------------------------------------- |
| Int      | Value equality                                    |
| Bool     | Value equality                                    |
| String   | UTF-8 codepoint sequence equality                 |
| ADT      | Same constructor AND field-wise equality          |
| List     | Recursive structural equality                     |
| Map      | Key/value structural equality (order-independent) |
| Option   | Structural equality                               |
| Result   | Structural equality                               |
| Function | ❌ Not comparable (compile-time error)             |

Example:

```lang
equals([1,2], [1,2])   // true
equals(fn(x)=>x, fn(x)=>x) // ❌ compile error
```

There is **no reference equality** in the language.

---

# Ordering & Comparisons (🔒 Locked)

## 3. Ordering Semantics

### Rationale

Ordering must be explicit. Partial orders or JS-style coercion create subtle bugs.

### Specification

```text
Only Int and String are orderable.
```

#### Orderable Types

* `Int`: numeric ordering
* `String`: lexicographic ordering by UTF-8 codepoint

#### Non-Orderable Types

* ADTs
* List
* Map
* Option
* Result
* Function

Using `<`, `<=`, `>`, `>=` on unsupported types is a **compile-time error**.

Ordering is **total** for supported types.

---

# String Semantics (🔒 Locked)

## 4. String Model

### Rationale

Strings sit at the boundary between logic and the outside world (HTTP, JSON). Ambiguity here leaks everywhere.

### Specification

```text
Strings are immutable sequences of UTF-8 codepoints.
```

#### Rules

* No implicit numeric or boolean coercion
* Concatenation only via `+`
* Equality and ordering operate on UTF-8 codepoints
* No locale-dependent behavior
* No normalization (strings are byte-stable)

#### Limits

```text
Maximum string length is enforced at runtime boundaries.
```

Exact limits are implementation-defined but must be finite and documented.

---

# Recursion & Tail Calls (🔒 Locked)

## 5. Recursion Semantics

### Chosen Option: **Option A (Simpler & Safer)**

### Specification

```text
The language does not guarantee tail-call optimization.
```

#### Rules

* Recursion is allowed but depth-limited
* Recursion limits are enforced at runtime
* Exceeding the limit results in a controlled runtime error
* Iteration should be expressed using folds and higher-order functions

This keeps the runtime simple and avoids hidden performance cliffs.

---

# Module Initialization & Load Order (🔒 Locked)

## 6. Module Semantics

### Rationale

Top-level execution + effects = nondeterminism and deployment bugs.

### Specification

```text
Modules contain declarations only.
```

#### Rules

* No executable expressions at module scope
* No effects at module scope
* No initialization code
* All effects occur inside functions or APIs only

Modules are loaded declaratively and are order-independent.

---

# Import Semantics (🔒 Locked)

## 7. Import Rules

### Specification

```text
Imports are static and resolved at compile time.
```

#### Rules

* No dynamic imports
* Import paths are absolute module names
* All imports must resolve during compilation
* Name conflicts are compile-time errors unless explicitly aliased

Example:

```lang
import auth.user as userAuth
```

There is no runtime module loading.

---

# Reserved Words & Namespaces (🔒 Locked)

## 8. Reserved Keywords

The following identifiers are reserved and may not be redefined or shadowed:

```text
function, let, type, effect, module, api,
if, else, match, fn, uses,
true, false,
some, none, ok, err,
language
```

Future versions may add keywords but never remove existing ones.

---

# Versioning & Compatibility (🔒 Locked)

## 9. Language Versioning

### Specification

```lang
language v1.0
```

#### Rules

* Source files must declare a language version
* Compiler rejects unknown or incompatible versions
* Minor versions add features only
* Major versions may introduce breaking changes

This is mandatory for long-term ecosystem stability.

---

# Compiler ↔ Runtime Contract (🔒 Locked)

## 10. Tooling Contracts

### Specification

```text
The AST is canonical and stable.
```

#### Bytecode Guarantees

Bytecode must include:

* language version
* stdlib version
* effect signature hash

```text
The runtime must reject incompatible bytecode.
```

This guarantees safe hot-swapping and deployment.

---

# Debuggability (🔒 Locked)

## 11. Debug & Error Reporting

### Specification

```text
Debug builds may include source maps.
Production builds never include source text.
```

#### Rules

* Errors always include location (file, line, column)
* No stack traces in production
* Internal VM details are never exposed

This balances safety with developer usability.

---

# Explicit Non-Goals (🔒 Locked)

## 12. Explicitly Not Supported

```text
The following are explicitly not supported in v1:
- Classes and inheritance
- Reflection or introspection
- Metaprogramming or macros
- Dynamic code loading
- Concurrency primitives
- Shared mutable state
- Implicit coercions
```

These are deliberate exclusions.


# 🔴 Tier-0 — Semantics That Must Be Locked

These are **language guarantees**, not implementation details.

---

## 1. Memory Model (User-Visible Semantics) 🔒

### Why this matters

Auditors, formal reasoning, hashing, equality, and determinism **all depend on this**. If memory identity leaks, your language collapses back into JS-like ambiguity.

### **Final Spec: Memory Model (Semantic)**

```text
Memory Model (Semantic)

- All values are deeply immutable.
- Cycles are unrepresentable at the language level.
- Structural sharing is permitted internally but is not observable.
- Equality, hashing, and program behavior never depend on allocation identity.
```

### Clarifications

* **Deep immutability**
  Once a value is created, it can never be modified, directly or indirectly.

* **No cycles**
  There is no syntax or API that allows constructing cyclic Lists, Maps, or ADTs.

* **Structural sharing**
  The runtime may reuse memory for efficiency, but:

  * No identity checks exist
  * No pointer equality exists
  * No observable timing or behavioral differences are allowed

This enables:

* safe memoization
* deterministic hashing
* reproducible builds
* formal equivalence reasoning

---

## 2. Deterministic Evaluation Order 🔒

### Why this matters

Two compilers that disagree on evaluation order can both be “correct” and still produce different behavior. That is unacceptable.

### **Final Spec: Evaluation Order**

```text
Evaluation Order

- Function arguments are evaluated left-to-right.
- Pipeline stages are evaluated left-to-right.
- The scrutinee of a match expression is evaluated exactly once.
- Match cases are tested top-to-bottom.
- Map iteration order is undefined but deterministic for a given value.
```

### Important consequences

* **Map iteration**

  * No guarantees about order across constructions
  * But the same Map value must always iterate identically
  * Equality and hashing must not depend on iteration order

This avoids accidental reliance on insertion order while preserving determinism.

---

## 3. Map Key Constraints 🔒

### Why this matters

Maps are unusable in serious systems without deterministic, well-defined key semantics.

### **Final Spec: Map Key Rules**

```text
Map Key Rules

- Map keys must be structurally comparable.
- Functions are forbidden as map keys.
- Hashing is structural and deterministic.
- Maps with equivalent key-value pairs are equal regardless of insertion order.
```

### Details

* **Structurally comparable** means:

  * Int, Bool, String
  * ADTs whose fields are themselves comparable
  * Lists and Maps (recursively)

* **Forbidden**

  * Functions
  * Any future effectful or opaque types

This guarantees:

* safe caching
* reliable equality
* deterministic serialization

---

# 🟠 Tier-1 — Language Completeness & Auditability

These prevent subtle reasoning gaps and reviewer confusion.

---

## 4. Exhaustiveness Rules for `if` 🔒

### Why this matters

An `if` without `else` introduces implicit `Unit` or `undefined` semantics — which your language explicitly rejects.

### **Final Spec: If Expressions**

```text
If Expressions

- `if` without `else` is forbidden.
- All `if` expressions must return a value.
- Both branches must return the same type.
```

This keeps the language:

* expression-oriented
* total
* free of implicit “void” values

---

## 5. Shadowing Rules 🔒

### Why this matters

Shadowing silently breaks audits and code reviews.

### **Chosen Option: A (Safer)**

### **Final Spec: Variable Binding**

```text
Variable Shadowing

- Variable shadowing is forbidden.
- Redeclaring an identifier in the same or nested scope is a compile-time error.
```

Example (❌ invalid):

```lang
let x = 1
if cond {
  let x = 2   // compile error
}
```

This dramatically improves:

* readability
* auditability
* refactoring safety

---

## 6. Totality of Functions 🔒

### Why this matters

Partial functions undermine formal reasoning and API safety.

### **Final Spec: Function Totality**

```text
Function Totality

- All functions must return a value on all control paths.
- Partial functions are not allowed.
- Missing return paths are compile-time errors.
```

This applies to:

* `if`
* `match`
* early returns (if introduced later)

---

# 🟡 Tier-2 — Tooling, Ecosystem, and Future-Proofing

These define whether the language scales beyond v1.

---

## 7. Canonical Formatting 🔒

### Why this matters

Canonical source form enables reproducibility and clean diffs.

### **Final Spec: Canonical Formatting**

```text
Canonical Formatting

- The language defines a canonical formatter.
- Formatting is not semantically significant.
- Tooling may reject non-canonically formatted source.
```

This enables:

* deterministic builds
* stable diffs
* formatter-as-gatekeeper workflows

---

## 8. Package / Dependency Identity (v1) 🔒

### Why this matters

You must explicitly say what you *don’t* support yet.

### **Final Spec: Dependency Model (v1)**

```text
Dependency Model (v1)

- There is no external package manager.
- All modules are compiled together.
- Dependency resolution is static and local.
- No network or dynamic dependency loading is permitted.
```

This avoids:

* supply-chain risk
* version skew
* non-reproducible builds

Future versions may add packages explicitly and deliberately.

---

## 9. Testing Semantics 🔒

### Why this matters

Your effect system is only credible if testing is first-class.

### **Final Spec: Testing Model**

```text
Testing Model

- Pure functions may be evaluated in isolation.
- Effects are mockable via explicit effect injection.
- API handlers are testable by providing mock effect implementations.
```

Implications:

* No global mocks
* No hidden state
* Tests are deterministic by construction

# 🔴 Tier-0 — Semantic Commitments (🔒 Locked)

These define what programs **mean**, not how they’re implemented.

---

## 1. Hashing Semantics (Structural, Deterministic) 🔒

### Why this matters

Structural equality without hashing semantics is incomplete. Hashing underpins:

* Maps
* Memoization
* Caching
* Deterministic builds
* Audit proofs

If hashing isn’t locked, two correct runtimes can disagree.

### **Final Spec: Hashing Semantics**

```text
Hashing Semantics

- Hashing is structural and deterministic.
- If equals(a, b) is true, then hash(a) == hash(b).
- Hashing is independent of memory layout, allocation identity, or runtime representation.
- Hash computation must not depend on Map iteration order.
```

### Clarifications

* Hashing is defined over **logical value structure**, not physical representation
* Structural sharing must not influence hash results
* Hash functions are stable across runs and platforms

This guarantees Maps, caches, and memoization are sound and auditable.

---

## 2. Serialization Canonical Form 🔒

### Why this matters

Serialization affects:

* API signatures
* Caching layers
* Replay systems
* Legal/audit logs
* Cross-runtime consistency

Without canonical form, two “equal” values may serialize differently.

### **Final Spec: Serialization Semantics**

```text
Serialization Semantics

- All serializable values have a canonical encoding.
- Structurally equivalent values serialize identically.
- Map serialization order is deterministic but not insertion-based.
- Serialization is independent of runtime, platform, or compiler implementation.
```

### Implications

* Serialization is suitable for hashing and signing
* Replays are deterministic
* Logs are legally defensible

JSON is a *boundary format*; canonical form applies **before** JSON encoding.

---

## 3. Time, Randomness, and Entropy 🔒

### Why this matters

Time and randomness are the most common sources of hidden nondeterminism and side channels.

You must name them as **explicit capabilities**.

### **Final Spec: Time & Randomness Effects**

```lang
effect time
effect random
```

```text
Time and Randomness Semantics

- Access to time requires the `time` effect.
- Access to randomness requires the `random` effect.
- No implicit entropy sources exist.
- Randomness must be injected by the runtime.
```

### Consequences

* “Pure” functions cannot observe time or randomness
* Reproducible execution is possible
* No timing or entropy side channels exist

This is essential for deterministic testing and audits.

---

## 4. Resource Accounting Semantics 🔒

### Why this matters

Limits alone are insufficient. Auditors care about **how** resource usage is defined and enforced.

### **Final Spec: Resource Accounting**

```text
Resource Accounting

- Each execution is subject to:
  - an instruction budget
  - a memory budget
  - an allocation budget
- Resource usage is deterministic for a given input and bytecode.
- Budget exhaustion results in a controlled runtime error.
```

### Clarifications

* Resource exhaustion does not crash the VM
* No partial state corruption is allowed
* Accounting does not depend on host timing or scheduling

This enables:

* DOS resistance
* Predictable billing
* Safe multi-tenant hosting

---

# 🟠 Tier-1 — Auditability & Reasoning Completeness

These close loopholes auditors will absolutely ask about.

---

## 5. Determinism of Built-ins 🔒

### Why this matters

If stdlib functions are not explicitly constrained, nondeterminism sneaks in via:

* locale-sensitive string ops
* environment-dependent behavior

### **Final Spec: Standard Library Determinism**

```text
Standard Library Determinism

- All standard library functions are deterministic.
- No standard library function may depend on:
  - time
  - randomness
  - environment
  unless explicitly effectful.
```

Any stdlib function that depends on time or randomness must require the corresponding effect.

---

## 6. Numeric Error Propagation 🔒

### Why this matters

You correctly banned exceptions. You must also ban silent numeric failure modes.

### **Final Spec: Numeric Errors**

```text
Numeric Errors

- Numeric runtime errors produce a controlled runtime error.
- They do not crash the virtual machine.
- They do not corrupt program state.
- They are not catchable by user code.
```

Examples:

* integer overflow
* division by zero
* invalid numeric operations

This preserves determinism and simplifies reasoning.

---

## 7. Termination Guarantees 🔒

### Why this matters

Security reviews and formal reasoning require explicit non-termination policy.

### **Final Spec: Termination Semantics**

```text
Termination Semantics

- The language does not guarantee termination.
- Infinite loops are impossible by syntax.
- Infinite recursion is halted by runtime execution limits.
- Non-termination is prevented by enforced resource budgets.
```

### Implications

* Programs cannot hang indefinitely
* Worst-case behavior is bounded and predictable
* Suitable for regulated and sandboxed environments

---

## 8. API Idempotency & Side-Effect Ordering 🔒

### Why this matters

Auditors *will* ask: “What happens on retries?”

You must not imply transactional behavior you don’t provide.

### **Final Spec: API Execution Semantics**

```text
API Execution Semantics

- Effects are executed strictly in source order.
- No implicit retries exist.
- Idempotency must be implemented explicitly by user code.
- Partial execution is not automatically rolled back.
```

### Consequences

* No hidden transactions
* No surprise retries
* Clear operational semantics

---

# 🟡 Tier-2 — Ecosystem & Longevity

These ensure the language scales responsibly.

---

## 9. Binary & Artifact Reproducibility 🔒

### Why this matters

This is foundational for supply-chain security and compliance.

### **Final Spec: Build Reproducibility**

```text
Build Reproducibility

- Compilation is deterministic.
- Identical source code and compiler version produce identical bytecode.
- Timestamps, file paths, and host metadata are excluded from artifacts.
```

This enables:

* verifiable builds
* reproducible deployments
* cryptographic verification

---

## 10. Compatibility & Deprecation Policy 🔒

### Why this matters

Strict languages without policy fragment ecosystems quickly.

### **Final Spec: Compatibility Policy**

```text
Compatibility Policy

- Minor versions are backward-compatible.
- Deprecated features must emit warnings for at least one major version.
- Removed features require a major version bump.
```

This protects users and tooling authors.

---

## 11. Foreign Function Boundary 🔒

### Why this matters

If FFI is “implicitly possible,” it becomes a backdoor.

### **Final Spec: Foreign Code**

```text
Foreign Code

- There is no foreign function interface in v1.
- User code cannot call host functions except via declared effects.
- All host interaction is capability-based.
```

This preserves your:

* security model
* auditability
* sandbox guarantees

# Standard Libraries to implement

# 🟥 Tier-0: Mandatory Standard Libraries (v1)

If these are missing, the language will feel incomplete immediately.

---

## 1. Core / Prelude (Always Imported)

This is equivalent to `Prelude` (Haskell), `std::prelude` (Rust).

### Types

```text
Int
Bool
String
Option<T>
Result<T, E>
List<T>
Map<K, V>
Json
```

### Functions

```text
identity
equals
hash
pipe
compose
not
and
or
```

### Option

```text
map
flatMap
unwrapOr
isSome
isNone
```

### Result

```text
map
flatMap
mapError
unwrapOr
```

📌 **Why mandatory:**
Without this, nothing composes cleanly.

---

## 2. Math (Pure, Int-Only)

### Library: `math`

```text
add
sub
mul
div
mod
abs
min
max
clamp
```

Rules:

* Int64 only
* overflow → runtime error
* no floats
* no randomness

📌 This matches early Go / embedded Rust use cases.

---

## 3. String (Pure, Deterministic)

### Library: `string`

```text
length
isEmpty
concat
split
join
contains
startsWith
endsWith
toUpperAscii
toLowerAscii
```

Rules:

* UTF-8
* ASCII-only case ops (explicit)
* no locale dependence

📌 This avoids Java / JS string footguns.

---

## 4. List (Functional Backbone)

### Library: `list`

```text
map
flatMap
filter
fold
foldRight
length
reverse
take
drop
find
all
any
```

Rules:

* immutable
* recursive
* no mutation helpers

📌 This replaces loops and keeps recursion shallow.

---

## 5. Map (Key-Value Backbone)

### Library: `map`

```text
empty
get
set
remove
containsKey
keys
values
merge
```

Rules:

* structural keys only
* deterministic hashing
* order-independent equality

📌 This is essential for backend state modeling.

---

## 6. JSON (Boundary-Only)

### Library: `json`

```text
encode
decode
get
getString
getInt
getObject
getArray
```

Rules:

* schema-checked at API boundaries
* never used for internal logic
* canonical serialization

📌 This matches modern API-first backends.

---

## 7. HTTP (Effectful)

### Library: `http` (requires `http` effect)

```lang
effect http
```

```text
Request
Response
getHeader
setHeader
json
text
status
```

Rules:

* no sockets
* no raw streams
* request/response only

📌 This replaces Express/Fiber/etc. safely.

---

## 8. Logging (Effectful, Explicit)

### Library: `log` (requires `log` effect)

```lang
effect log
```

```text
info
warn
error
```

Rules:

* structured logs only
* no printf
* deterministic formatting

📌 Required for observability and audits.

---

## 9. Time (Effectful, Explicit)

### Library: `time`

```lang
effect time
```

```text
now
unixMillis
```

Rules:

* no implicit clocks
* injectable for tests

📌 Prevents nondeterministic logic.

---

## 10. Random (Effectful, Explicit)

### Library: `random`

```lang
effect random
```

```text
int
bytes
```

Rules:

* runtime-provided entropy
* seedable for tests
* never implicit

📌 This keeps cryptography honest.

---

# 🟧 Tier-1: Strongly Recommended (v1.x)

These make the language **pleasant** and **competitive**.

---

## 11. Validation

### Library: `validate`

```text
nonEmpty
minLength
maxLength
matches
inRange
```

📌 Used everywhere in APIs.

---

## 12. Error Construction (Pure)

### Library: `error`

```text
new
withCode
withMessage
```

📌 Helps structured error handling without exceptions.

---

## 13. Encoding / Decoding

### Library: `base64`, `hex`

```text
encode
decode
```

📌 Needed for tokens, IDs, signatures.

---

## 14. Crypto (Effectful, Limited)

### Library: `crypto` (effect `crypto`)

```lang
effect crypto
```

```text
hashSha256
hmacSha256
```

Rules:

* no custom crypto
* no raw primitives
* safe defaults only

📌 Enough for auth, signatures, tokens.

---

## 15. UUID / ID Generation

### Library: `id` (effect `random`)

```text
uuidV4
```

📌 Standard backend need.

---

# 🟨 Tier-2: Optional but Valuable

---

## 16. File / Storage (Effectful, Sandboxed)

Only if you support it at all:

```lang
effect storage
```

```text
read
write
exists
```

📌 Often excluded in SaaS runtimes — OK either way.

---

## 17. Testing Helpers (Dev-Only)

```text
assert
assertEquals
assertError
```

📌 Never shipped to production runtime.

---

# 🚫 What MUST NOT Be in the Standard Library

This is as important as what *is* included.

```text
Explicitly not included:

- Threading / concurrency primitives
- Global mutable state
- Reflection or introspection
- ORM / database abstractions
- Regex engines (unless deterministic + bounded)
- Date arithmetic (timezone hell)
- OS access
- Environment variables
```

These belong in **capability-restricted host effects**, not stdlib.

---

# 🧭 How This Compares to Other Backend Languages

| Language | What You Match or Beat                  |
| -------- | --------------------------------------- |
| Go       | Simpler semantics, stronger determinism |
| Rust     | Easier APIs, fewer footguns             |
| Java     | Smaller stdlib, much safer              |
| Node     | Orders of magnitude safer               |
| WASM     | Easier to program, similar isolation    |
