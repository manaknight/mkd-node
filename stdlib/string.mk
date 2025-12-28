language v1.0

module string

// String Operations - UTF-8 string manipulation

function length(s: String): Int {
  // Implementation provided by runtime
  0  // Placeholder - runtime counts UTF-8 codepoints
}

function isEmpty(s: String): Bool {
  length(s) == 0
}

function concat(a: String, b: String): String {
  // Implementation provided by runtime
  ""  // Placeholder - runtime concatenates UTF-8 strings
}

function split(s: String, separator: String): List<String> {
  // Implementation provided by runtime
  // Returns list of substrings split by separator
  nil  // Placeholder
}

function join(strings: List<String>, separator: String): String {
  // Implementation provided by runtime
  // Joins list of strings with separator
  ""  // Placeholder
}

function contains(s: String, substring: String): Bool {
  // Implementation provided by runtime
  false  // Placeholder
}

function startsWith(s: String, prefix: String): Bool {
  // Implementation provided by runtime
  false  // Placeholder
}

function endsWith(s: String, suffix: String): Bool {
  // Implementation provided by runtime
  false  // Placeholder
}

function toUpperAscii(s: String): String {
  // Implementation provided by runtime
  // Converts ASCII letters to uppercase, preserves others
  s  // Placeholder
}

function toLowerAscii(s: String): String {
  // Implementation provided by runtime
  // Converts ASCII letters to lowercase, preserves others
  s  // Placeholder
}

function substring(s: String, start: Int, length: Int): Result<String, String> {
  let str_len = length(s)
  if start < 0 || length < 0 || start >= str_len {
    err("invalid substring parameters")
  } else {
    let end = min(start + length, str_len)
    ok(substringUnchecked(s, start, end - start))
  }
}

// Helper function (implementation provided by runtime)
function substringUnchecked(s: String, start: Int, length: Int): String {
  s  // Placeholder
}

function indexOf(s: String, substring: String): Option<Int> {
  // Implementation provided by runtime
  // Returns index of first occurrence, or none
  none  // Placeholder
}

function lastIndexOf(s: String, substring: String): Option<Int> {
  // Implementation provided by runtime
  // Returns index of last occurrence, or none
  none  // Placeholder
}

function replace(s: String, old_substring: String, new_substring: String): String {
  // Implementation provided by runtime
  // Replaces all occurrences of old_substring with new_substring
  s  // Placeholder
}

function replaceFirst(s: String, old_substring: String, new_substring: String): String {
  // Implementation provided by runtime
  // Replaces first occurrence of old_substring with new_substring
  s  // Placeholder
}

function trim(s: String): String {
  // Implementation provided by runtime
  // Removes leading and trailing whitespace
  s  // Placeholder
}

function trimStart(s: String): String {
  // Implementation provided by runtime
  // Removes leading whitespace
  s  // Placeholder
}

function trimEnd(s: String): String {
  // Implementation provided by runtime
  // Removes trailing whitespace
  s  // Placeholder
}

function padStart(s: String, target_length: Int, pad_string: String): String {
  // Implementation provided by runtime
  // Pads start of string to reach target_length using pad_string
  s  // Placeholder
}

function padEnd(s: String, target_length: Int, pad_string: String): String {
  // Implementation provided by runtime
  // Pads end of string to reach target_length using pad_string
  s  // Placeholder
}

function repeat(s: String, count: Int): Result<String, String> {
  if count < 0 {
    err("negative repeat count")
  } else {
    ok(repeatUnchecked(s, count))
  }
}

// Helper function (implementation provided by runtime)
function repeatUnchecked(s: String, count: Int): String {
  s  // Placeholder
}

// Comparison functions

function equals(a: String, b: String): Bool {
  // Implementation provided by runtime - UTF-8 codepoint comparison
  false  // Placeholder
}

function compare(a: String, b: String): Int {
  // Implementation provided by runtime - lexicographic UTF-8 comparison
  // Returns -1 if a < b, 0 if equal, 1 if a > b
  0  // Placeholder
}

function isLessThan(a: String, b: String): Bool {
  compare(a, b) < 0
}

function isGreaterThan(a: String, b: String): Bool {
  compare(a, b) > 0
}

// Conversion functions

function fromInt(n: Int): String {
  // Implementation provided by runtime
  // Converts integer to decimal string representation
  "0"  // Placeholder
}

function toInt(s: String): Result<Int, String> {
  // Implementation provided by runtime
  // Parses decimal string to integer
  err("not implemented")  // Placeholder
}

// Character operations (simplified - single UTF-8 codepoints)

function charAt(s: String, index: Int): Result<String, String> {
  let str_len = length(s)
  if index < 0 || index >= str_len {
    err("index out of bounds")
  } else {
    ok(charAtUnchecked(s, index))
  }
}

// Helper function (implementation provided by runtime)
function charAtUnchecked(s: String, index: Int): String {
  ""  // Placeholder - returns single codepoint as string
}

// String formatting (basic)

function format(template: String, args: List<String>): String {
  // Implementation provided by runtime
  // Simple string formatting with {0}, {1}, etc. placeholders
  template  // Placeholder
}
