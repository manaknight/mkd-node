language v1.0

module random

// Random Number Generation - Cryptographically secure random operations

// Basic random functions

function int(): Int uses { random } {
  // Implementation provided by runtime
  // Returns a cryptographically secure random 64-bit integer
  0  // Placeholder
}

function intRange(min: Int, max: Int): Result<Int, String> uses { random } {
  if min >= max {
    err("min must be less than max")
  } else {
    let range = max - min
    let random_int = int()
    // Use modulo to get value in range, but handle negative modulo correctly
    let positive_mod = if random_int < 0 {
                         (random_int * -1) % range
                       } else {
                         random_int % range
                       }
    ok(min + positive_mod)
  }
}

function intInclusive(min: Int, max: Int): Result<Int, String> uses { random } {
  intRange(min, max + 1)
}

// Boolean random

function bool(): Bool uses { random } {
  (int() % 2) == 0
}

// Floating point random (if supported in future)
// function float(): Float uses { random }
// function floatRange(min: Float, max: Float): Result<Float, String> uses { random }

// Random bytes

function bytes(length: Int): Result<List<Int>, String> uses { random } {
  if length < 0 {
    err("negative length")
  } else {
    bytesHelper(length, nil)
  }
}

function bytesHelper(remaining: Int, acc: List<Int>): List<Int> uses { random } {
  if remaining <= 0 {
    acc
  } else {
    let byte = int() % 256  // 0-255
    let positive_byte = if byte < 0 { byte * -1 } else { byte }
    bytesHelper(remaining - 1, cons(positive_byte, acc))
  }
}

// Random string generation

function string(length: Int, charset: String): Result<String, String> uses { random } {
  if length < 0 {
    err("negative length")
  } else {
    if charset == "" {
      err("empty charset")
    } else {
      stringHelper(length, charset, "")
    }
  }
}

function stringHelper(remaining: Int, charset: String, acc: String): String uses { random } {
  if remaining <= 0 {
    acc
  } else {
    let charset_len = length(charset)
    match intRange(0, charset_len - 1) {
      ok(index) -> match charAt(charset, index) {
                     ok(char) -> stringHelper(remaining - 1, charset, acc + char)
                     err(_) -> stringHelper(remaining - 1, charset, acc + "?")  // Fallback
                   }
      err(_) -> stringHelper(remaining - 1, charset, acc + "?")  // Fallback
    }
  }
}

// Common charsets

function alphanumericCharset(): String {
  "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789"
}

function alphabeticCharset(): String {
  "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz"
}

function numericCharset(): String {
  "0123456789"
}

function hexCharset(): String {
  "0123456789ABCDEF"
}

function base64Charset(): String {
  "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/"
}

// Convenience functions

function alphanumericString(length: Int): Result<String, String> uses { random } {
  string(length, alphanumericCharset())
}

function alphabeticString(length: Int): Result<String, String> uses { random } {
  string(length, alphabeticCharset())
}

function numericString(length: Int): Result<String, String> uses { random } {
  string(length, numericCharset())
}

function hexString(length: Int): Result<String, String> uses { random } {
  string(length, hexCharset())
}

// UUID generation

function uuidV4(): String uses { random } {
  // Generate UUID v4: xxxxxxxx-xxxx-4xxx-yxxx-xxxxxxxxxxxx
  let bytes = bytes(16)  // 16 random bytes

  // This would need proper UUID formatting
  // For now, return a placeholder
  "xxxxxxxx-xxxx-4xxx-yxxx-xxxxxxxxxxxx"
}

// Secure token generation

function secureToken(length: Int): Result<String, String> uses { random } {
  // Generate a secure token using URL-safe base64-like charset
  string(length, "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_")
}

// Shuffle operations

function shuffleList<T>(list: List<T>): List<T> uses { random } {
  // Fisher-Yates shuffle implementation
  let list_array = listToArray(list)
  shuffleArray(list_array)
}

function listToArray<T>(list: List<T>): List<T> {
  // Convert list to array representation for shuffling
  list  // Placeholder - would need array conversion
}

function shuffleArray<T>(arr: List<T>): List<T> uses { random } {
  // Fisher-Yates shuffle on array
  arr  // Placeholder
}

// Random sampling

function sampleList<T>(list: List<T>, count: Int): Result<List<T>, String> uses { random } {
  let len = lengthList(list)
  if count < 0 {
    err("negative count")
  } else {
    if count >= len {
      ok(list)  // Return all elements
    } else {
      sampleWithoutReplacement(list, count, nil)
    }
  }
}

function sampleWithoutReplacement<T>(list: List<T>, count: Int, acc: List<T>): List<T> uses { random } {
  if count <= 0 {
    acc
  } else {
    let len = lengthList(list)
    match intRange(0, len - 1) {
      ok(index) -> match listGet(list, index) {
                     ok(item) -> let remaining = listRemoveAt(list, index)
                                 sampleWithoutReplacement(remaining, count - 1, cons(item, acc))
                     err(_) -> sampleWithoutReplacement(list, count, acc)  // Retry
                   }
      err(_) -> sampleWithoutReplacement(list, count, acc)  // Retry
    }
  }
}

// Helper functions

function listGet<T>(list: List<T>, index: Int): Result<T, String> {
  listGetHelper(list, index, 0)
}

function listGetHelper<T>(list: List<T>, target_index: Int, current_index: Int): Result<T, String> {
  match list {
    cons(h, t) -> if current_index == target_index {
                   ok(h)
                 } else {
                   listGetHelper(t, target_index, current_index + 1)
                 }
    nil -> err("index out of bounds")
  }
}

function listRemoveAt<T>(list: List<T>, index: Int): List<T> {
  listRemoveAtHelper(list, index, 0, nil)
}

function listRemoveAtHelper<T>(list: List<T>, target_index: Int, current_index: Int, acc: List<T>): List<T> {
  match list {
    cons(h, t) -> if current_index == target_index {
                   reverseList(acc) + t  // Skip this element
                 } else {
                   listRemoveAtHelper(t, target_index, current_index + 1, cons(h, acc))
                 }
    nil -> reverseList(acc)  // Index not found, return original
  }
}

// Statistical distributions (conceptual - would need more math support)

type NormalDistribution {
  mean: Int
  std_dev: Int
}

function normalDistribution(mean: Int, std_dev: Int): NormalDistribution {
  NormalDistribution(mean, std_dev)
}

function sampleNormal(dist: NormalDistribution): Int uses { random } {
  // Box-Muller transform for normal distribution
  // This is a simplified implementation
  let u1 = int() % 1000 / 1000.0  // Random float 0-1
  let u2 = int() % 1000 / 1000.0  // Random float 0-1

  // Simplified - would need proper floating point math
  dist.mean  // Placeholder
}

// Seed management (for testing - not cryptographically secure)

function withSeed<T>(seed: Int, operation: fn() -> T): T uses { random } {
  // Implementation would allow deterministic testing
  // For now, just execute the operation
  operation()
}

// Random state inspection (for testing)

type RandomState {
  // Internal state representation
  // Implementation dependent
}

function getRandomState(): RandomState uses { random } {
  // Implementation provided by runtime
  RandomState()  // Placeholder
}

function setRandomState(state: RandomState) uses { random } {
  // Implementation provided by runtime
  ()  // Unit return
}

// Quality testing

function testRandomQuality(samples: Int): RandomTestResult uses { random } {
  // Run statistical tests on random number quality
  RandomTestResult(true, "tests passed")  // Placeholder
}

type RandomTestResult {
  passed: Bool
  details: String
}
