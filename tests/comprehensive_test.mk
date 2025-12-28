// Comprehensive test covering IMPLEMENTATION_TASKS.md features

// Task 1.1: AST Definitions - Functions, literals, expressions
fn add(x: Int64, y: Int64) -> Int64 {
    x + y
}

fn multiply(a: Int64, b: Int64) -> Int64 {
    a * b
}

// Task 1.2: Lexer - Keywords, operators, literals
fn test_literals() -> String {
    let num = 42
    let str = "Hello"
    let bool_val = true

    if bool_val {
        str + " World " + num.toString()
    } else {
        "false case"
    }
}

// Task 1.3: Parser - API routes, function declarations
api get "/test/:id" (id: String) -> String {
    "API route with param: " + id
}

api post "/data" () -> String {
    "POST request"
}

// Task 2.2: Type Checker - Type safety, control flow
fn test_types() -> Int64 {
    let x = 10
    let y = 20
    let result = add(x, y)

    if result > 25 {
        result
    } else {
        0
    }
}

// Task 2.3: Effect Analyzer - Effect declarations (would need effects.mk)
effect log
fn test_effects() -> String {
    log.info("Testing effects")
    "Effects work"
}

// Task 3.1: IR Lowering - Pipelines, expressions
fn test_pipeline() -> Int64 {
    5 |> add(10) |> multiply(2)
}

// Task 3.3: JS Emitter - All constructs compile to valid JS
fn main() -> String {
    let greeting = test_literals()
    let calc = test_types()
    let piped = test_pipeline()

    greeting + " | Calc: " + calc.toString() + " | Pipeline: " + piped.toString()
}
