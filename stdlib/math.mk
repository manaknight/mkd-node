language v1.0

module math

// Math Operations - Pure integer arithmetic with overflow checking

function add(a: Int, b: Int): Int {
  // Implementation provided by runtime with overflow checking
  // This will trap on overflow rather than wrapping
  a + b  // Placeholder - runtime implements checked addition
}

function sub(a: Int, b: Int): Int {
  // Implementation provided by runtime with overflow checking
  a - b  // Placeholder - runtime implements checked subtraction
}

function mul(a: Int, b: Int): Int {
  // Implementation provided by runtime with overflow checking
  a * b  // Placeholder - runtime implements checked multiplication
}

function div(a: Int, b: Int): Result<Int, String> {
  if b == 0 {
    err("division by zero")
  } else {
    ok(a / b)  // Integer division truncates toward zero
  }
}

function mod(a: Int, b: Int): Result<Int, String> {
  if b == 0 {
    err("modulo by zero")
  } else {
    ok(a % b)  // Modulo with same sign as dividend
  }
}

function abs(x: Int): Int {
  if x < 0 {
    0 - x  // Use sub to ensure overflow checking
  } else {
    x
  }
}

function min(a: Int, b: Int): Int {
  if a < b {
    a
  } else {
    b
  }
}

function max(a: Int, b: Int): Int {
  if a > b {
    a
  } else {
    b
  }
}

function clamp(value: Int, min_val: Int, max_val: Int): Int {
  min(max(value, min_val), max_val)
}

function sign(x: Int): Int {
  if x > 0 {
    1
  } else {
    if x < 0 {
      -1
    } else {
      0
    }
  }
}

function pow(base: Int, exponent: Int): Result<Int, String> {
  if exponent < 0 {
    err("negative exponent not supported")
  } else {
    powHelper(base, exponent, 1)
  }
}

// Helper function for exponentiation
function powHelper(base: Int, exp: Int, acc: Int): Int {
  if exp == 0 {
    acc
  } else {
    powHelper(base, exp - 1, mul(acc, base))
  }
}

// Comparison functions (though <, >, <=, >= are built-in)

function compare(a: Int, b: Int): Int {
  if a < b {
    -1
  } else {
    if a > b {
      1
    } else {
      0
    }
  }
}

function equals(a: Int, b: Int): Bool {
  a == b
}

// Factorial (for demonstration - would overflow quickly)
function factorial(n: Int): Result<Int, String> {
  if n < 0 {
    err("factorial of negative number")
  } else {
    ok(factorialHelper(n, 1))
  }
}

function factorialHelper(n: Int, acc: Int): Int {
  if n <= 1 {
    acc
  } else {
    factorialHelper(n - 1, mul(acc, n))
  }
}

// Greatest common divisor
function gcd(a: Int, b: Int): Int {
  if b == 0 {
    abs(a)
  } else {
    gcd(b, mod(a, b))
  }
}

// Least common multiple
function lcm(a: Int, b: Int): Result<Int, String> {
  if a == 0 || b == 0 {
    ok(0)
  } else {
    div(mul(abs(a), abs(b)), gcd(a, b))
  }
}
