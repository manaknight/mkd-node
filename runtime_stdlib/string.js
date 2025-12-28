// Manaknight Standard Library - String Implementation
// UTF-8 aware string operations

"use strict";

// Length in UTF-8 codepoints
function length(s) {
  if (typeof s !== 'string') return 0;

  // Count UTF-8 codepoints (not UTF-16 code units)
  let count = 0;
  for (let i = 0; i < s.length; i++) {
    const code = s.charCodeAt(i);
    // Count surrogate pairs as single codepoints
    if (code >= 0xD800 && code <= 0xDBFF) {
      i++; // Skip low surrogate
    }
    count++;
  }
  return count;
}

// Check if string is empty
function isEmpty(s) {
  return s === "";
}

// Concatenate strings
function concat(a, b) {
  return a + b;
}

// Split string by separator
function split(s, separator) {
  if (separator === "") {
    // Split into individual characters
    const chars = [];
    for (let i = 0; i < s.length; i++) {
      const code = s.charCodeAt(i);
      if (code >= 0xD800 && code <= 0xDBFF && i + 1 < s.length) {
        // Surrogate pair
        chars.push(s.substring(i, i + 2));
        i++; // Skip low surrogate
      } else {
        chars.push(s.charAt(i));
      }
    }
    return arrayToList(chars.reverse());
  }

  const parts = s.split(separator);
  return arrayToList(parts.reverse());
}

// Join array of strings with separator
function join(strings, separator) {
  const arr = listToArray(strings);
  return arr.join(separator);
}

// Check if string contains substring
function contains(s, substring) {
  return s.includes(substring);
}

// Check if string starts with prefix
function startsWith(s, prefix) {
  return s.startsWith(prefix);
}

// Check if string ends with suffix
function endsWith(s, suffix) {
  return s.endsWith(suffix);
}

// ASCII case conversion (explicit and limited)
function toUpperAscii(s) {
  let result = "";
  for (let i = 0; i < s.length; i++) {
    const c = s.charAt(i);
    const code = c.charCodeAt(0);
    if (code >= 97 && code <= 122) { // a-z
      result += String.fromCharCode(code - 32);
    } else {
      result += c;
    }
  }
  return result;
}

function toLowerAscii(s) {
  let result = "";
  for (let i = 0; i < s.length; i++) {
    const c = s.charAt(i);
    const code = c.charCodeAt(0);
    if (code >= 65 && code <= 90) { // A-Z
      result += String.fromCharCode(code + 32);
    } else {
      result += c;
    }
  }
  return result;
}

// Substring with bounds checking
function substring(s, start, length) {
  const str_len = s.length; // Use UTF-16 length for bounds checking
  if (start < 0 || length < 0 || start >= str_len) {
    return { tag: 'err', error: 'invalid substring parameters' };
  }

  const end = Math.min(start + length, str_len);
  return { tag: 'ok', value: s.substring(start, end) };
}

// Index of substring
function indexOf(s, substring) {
  const index = s.indexOf(substring);
  if (index === -1) {
    return { tag: 'none' };
  }
  return { tag: 'some', value: index };
}

// Last index of substring
function lastIndexOf(s, substring) {
  const index = s.lastIndexOf(substring);
  if (index === -1) {
    return { tag: 'none' };
  }
  return { tag: 'some', value: index };
}

// Replace all occurrences
function replace(s, old_substring, new_substring) {
  return s.split(old_substring).join(new_substring);
}

// Replace first occurrence
function replaceFirst(s, old_substring, new_substring) {
  const index = s.indexOf(old_substring);
  if (index === -1) return s;

  return s.substring(0, index) + new_substring + s.substring(index + old_substring.length);
}

// Trim whitespace
function trim(s) {
  return s.trim();
}

function trimStart(s) {
  return s.trimStart();
}

function trimEnd(s) {
  return s.trimEnd();
}

// Pad strings
function padStart(s, target_length, pad_string) {
  if (pad_string === "") pad_string = " ";
  while (s.length < target_length) {
    s = pad_string + s;
  }
  return s;
}

function padEnd(s, target_length, pad_string) {
  if (pad_string === "") pad_string = " ";
  while (s.length < target_length) {
    s = s + pad_string;
  }
  return s;
}

// Repeat string
function repeat(s, count) {
  if (count < 0) {
    return { tag: 'err', error: 'negative repeat count' };
  }

  let result = "";
  for (let i = 0; i < count; i++) {
    result += s;
  }
  return { tag: 'ok', value: result };
}

// Helper for repeat
function repeatUnchecked(s, count) {
  let result = "";
  for (let i = 0; i < count; i++) {
    result += s;
  }
  return result;
}

// Character access
function charAt(s, index) {
  const str_len = length(s); // Codepoint length
  if (index < 0 || index >= str_len) {
    return { tag: 'err', error: 'index out of bounds' };
  }

  // Convert codepoint index to UTF-16 index
  let utf16_index = 0;
  let codepoint_count = 0;

  while (codepoint_count < index && utf16_index < s.length) {
    const code = s.charCodeAt(utf16_index);
    if (code >= 0xD800 && code <= 0xDBFF) {
      utf16_index += 2; // Skip surrogate pair
    } else {
      utf16_index += 1;
    }
    codepoint_count++;
  }

  if (utf16_index >= s.length) {
    return { tag: 'err', error: 'index out of bounds' };
  }

  const code = s.charCodeAt(utf16_index);
  let char;
  if (code >= 0xD800 && code <= 0xDBFF && utf16_index + 1 < s.length) {
    char = s.substring(utf16_index, utf16_index + 2);
  } else {
    char = s.charAt(utf16_index);
  }

  return { tag: 'ok', value: char };
}

// Helper for charAt
function charAtUnchecked(s, index) {
  // Simplified version without bounds checking
  let utf16_index = 0;
  let codepoint_count = 0;

  while (codepoint_count < index && utf16_index < s.length) {
    const code = s.charCodeAt(utf16_index);
    if (code >= 0xD800 && code <= 0xDBFF) {
      utf16_index += 2;
    } else {
      utf16_index += 1;
    }
    codepoint_count++;
  }

  const code = s.charCodeAt(utf16_index);
  if (code >= 0xD800 && code <= 0xDBFF && utf16_index + 1 < s.length) {
    return s.substring(utf16_index, utf16_index + 2);
  } else {
    return s.charAt(utf16_index);
  }
}

// String comparison
function equals(a, b) {
  return a === b;
}

function compare(a, b) {
  if (a < b) return -1;
  if (a > b) return 1;
  return 0;
}

function isLessThan(a, b) {
  return a < b;
}

function isGreaterThan(a, b) {
  return a > b;
}

// Conversion functions
function fromInt(n) {
  return n.toString();
}

function toInt(s) {
  const num = parseInt(s, 10);
  if (isNaN(num)) {
    return { tag: 'err', error: 'invalid integer format' };
  }
  return { tag: 'ok', value: num };
}

// Template formatting
function format(template, args) {
  let result = template;
  const argArray = listToArray(args);

  for (let i = 0; i < argArray.length; i++) {
    const placeholder = `{${i}}`;
    result = result.split(placeholder).join(argArray[i]);
  }

  return result;
}

// List utility functions (from core.js - simplified versions)
function listToArray(list) {
  const arr = [];
  let current = list;
  while (current.tag === 'cons') {
    arr.push(current.head);
    current = current.tail;
  }
  return arr;
}

function arrayToList(arr) {
  let result = { tag: 'nil' };
  for (let i = arr.length - 1; i >= 0; i--) {
    result = { tag: 'cons', head: arr[i], tail: result };
  }
  return result;
}

// Export functions
if (typeof module !== 'undefined' && module.exports) {
  module.exports = {
    length,
    isEmpty,
    concat,
    split,
    join,
    contains,
    startsWith,
    endsWith,
    toUpperAscii,
    toLowerAscii,
    substring,
    indexOf,
    lastIndexOf,
    replace,
    replaceFirst,
    trim,
    trimStart,
    trimEnd,
    padStart,
    padEnd,
    repeat,
    repeatUnchecked,
    charAt,
    charAtUnchecked,
    equals,
    compare,
    isLessThan,
    isGreaterThan,
    fromInt,
    toInt,
    format
  };
}
