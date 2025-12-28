// Manaknight Standard Library - Math Implementation
// Checked integer arithmetic with overflow protection

"use strict";

// Constants for Int64 range checking
const INT64_MIN = -9223372036854775808n;
const INT64_MAX = 9223372036854775807n;

// Convert number to BigInt for safe operations
function toBigInt(n) {
  return BigInt(n);
}

// Convert BigInt back to number, checking range
function fromBigInt(bi) {
  if (bi < INT64_MIN || bi > INT64_MAX) {
    throw new Error("Integer overflow");
  }
  return Number(bi);
}

// Checked addition
function add(a, b) {
  const result = toBigInt(a) + toBigInt(b);
  return fromBigInt(result);
}

// Checked subtraction
function sub(a, b) {
  const result = toBigInt(a) - toBigInt(b);
  return fromBigInt(result);
}

// Checked multiplication
function mul(a, b) {
  const result = toBigInt(a) * toBigInt(b);
  return fromBigInt(result);
}

// Checked division with Result return
function div(a, b) {
  if (b === 0) {
    return { tag: 'err', error: 'division by zero' };
  }

  // Integer division with truncation toward zero
  const result = toBigInt(a) / toBigInt(b);
  return { tag: 'ok', value: fromBigInt(result) };
}

// Checked modulo with Result return
function mod(a, b) {
  if (b === 0) {
    return { tag: 'err', error: 'modulo by zero' };
  }

  // Modulo with same sign as dividend
  const result = toBigInt(a) % toBigInt(b);
  return { tag: 'ok', value: fromBigInt(result) };
}

// Absolute value
function abs(x) {
  return x < 0 ? sub(0, x) : x;
}

// Minimum and maximum
function min(a, b) {
  return a < b ? a : b;
}

function max(a, b) {
  return a > b ? a : b;
}

// Clamp value to range
function clamp(value, min_val, max_val) {
  if (value < min_val) return min_val;
  if (value > max_val) return max_val;
  return value;
}

// Sign function
function sign(x) {
  if (x > 0) return 1;
  if (x < 0) return -1;
  return 0;
}

// Comparison function
function compare(a, b) {
  if (a < b) return -1;
  if (a > b) return 1;
  return 0;
}

// Equality
function equals(a, b) {
  return a === b;
}

// Power function with integer exponent
function pow(base, exponent) {
  if (exponent < 0) {
    return { tag: 'err', error: 'negative exponent not supported' };
  }

  let result = 1;
  let current_base = base;
  let exp = exponent;

  while (exp > 0) {
    if (exp % 2 === 1) {
      result = mul(result, current_base);
    }
    current_base = mul(current_base, current_base);
    exp = Math.floor(exp / 2);
  }

  return { tag: 'ok', value: result };
}

// Helper function for power (used by pow)
function powHelper(base, exp, acc) {
  if (exp === 0) return acc;
  return powHelper(base, exp - 1, mul(acc, base));
}

// Greatest common divisor
function gcd(a, b) {
  a = abs(a);
  b = abs(b);

  while (b !== 0) {
    const temp = b;
    const mod_result = mod(a, b);
    if (mod_result.tag === 'err') return 0; // Should not happen
    b = mod_result.value;
    a = temp;
  }

  return a;
}

// Least common multiple
function lcm(a, b) {
  if (a === 0 || b === 0) {
    return { tag: 'ok', value: 0 };
  }

  const gcd_val = gcd(a, b);
  const product = mul(abs(a), abs(b));
  const div_result = div(product, gcd_val);

  return div_result;
}

// Factorial (limited to prevent overflow)
function factorial(n) {
  if (n < 0) {
    return { tag: 'err', error: 'factorial of negative number' };
  }

  if (n > 20) { // 21! is too big for Int64
    return { tag: 'err', error: 'factorial too large for Int64' };
  }

  let result = 1;
  for (let i = 2; i <= n; i++) {
    result = mul(result, i);
  }

  return { tag: 'ok', value: result };
}

// Factorial helper
function factorialHelper(n, acc) {
  if (n <= 1) return acc;
  return factorialHelper(n - 1, mul(acc, n));
}

// Export functions
if (typeof module !== 'undefined' && module.exports) {
  module.exports = {
    add,
    sub,
    mul,
    div,
    mod,
    abs,
    min,
    max,
    clamp,
    sign,
    compare,
    equals,
    pow,
    powHelper,
    gcd,
    lcm,
    factorial,
    factorialHelper
  };
}
