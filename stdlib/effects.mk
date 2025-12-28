language v1.0

module effects

// Effect Declarations - Define the capabilities available to Manaknight programs

// Time effect - for accessing current time and date operations
effect time

// Random effect - for generating random numbers and entropy
effect random

// HTTP effect - for making HTTP requests and responses
effect http

// Database effect - for database operations
effect db

// Logging effect - for writing log messages
effect log

// File system effect - for file operations (optional - not in minimal runtime)
effect fs

// Crypto effect - for cryptographic operations
effect crypto

// Network effect - for low-level network operations (distinct from HTTP)
effect net

// Environment effect - for accessing environment variables (restricted)
effect env

// System effect - for system-level operations (very restricted)
effect sys

// Effect Interfaces - Define the operations available for each effect
// These serve as documentation and type hints for the runtime

// Time effect operations
// function now(): Int uses { time }          // Current Unix timestamp in milliseconds
// function unixMillis(): Int uses { time }   // Current Unix timestamp in milliseconds
// function sleep(ms: Int): () uses { time }  // Sleep for milliseconds

// Random effect operations
// function int(): Int uses { random }           // Random 64-bit integer
// function intRange(min: Int, max: Int): Int uses { random }  // Random int in range
// function bytes(length: Int): List<Int> uses { random }      // Random bytes

// HTTP effect operations
// function get(url: String): Result<String, String> uses { http }              // HTTP GET
// function post(url: String, body: String): Result<String, String> uses { http } // HTTP POST
// function put(url: String, body: String): Result<String, String> uses { http }  // HTTP PUT
// function delete(url: String): Result<String, String> uses { http }             // HTTP DELETE
// function head(url: String): Result<String, String> uses { http }               // HTTP HEAD

// Database effect operations (generic interface)
// function query(sql: String, params: List<String>): Result<List<Map<String, String>>, String> uses { db }
// function execute(sql: String, params: List<String>): Result<Int, String> uses { db }

// Log effect operations
// function info(message: String) uses { log }
// function warn(message: String) uses { log }
// function error(message: String) uses { log }
// function debug(message: String) uses { log }

// File system effect operations (optional)
// function readFile(path: String): Result<String, String> uses { fs }
// function writeFile(path: String, content: String): Result<(), String> uses { fs }
// function exists(path: String): Bool uses { fs }

// Crypto effect operations
// function hashSha256(data: String): String uses { crypto }
// function hmacSha256(key: String, data: String): String uses { crypto }

// Environment effect operations (restricted)
// function getEnv(key: String): Option<String> uses { env }
// function setEnv(key: String, value: String) uses { env }

// System effect operations (very restricted)
// function exit(code: Int) uses { sys }
// function getPid(): Int uses { sys }

// Effect Composition Patterns

// Common effect combinations
// type WebAppEffects = { http, log, db, time }
// type CliToolEffects = { fs, log, time }
// type DataProcessorEffects = { db, log, crypto }

// Effect-safe wrapper functions
// These demonstrate how to write functions that use effects safely

function logAndReturn<T>(message: String, value: T): T uses { log } {
  // Log the operation and return the value
  // info(message)  // Would be called if we had the function
  value
}

function timeOperation<T>(operation: fn() -> T): T uses { time, log } {
  // Time an operation and log the duration
  let start = 0  // now()
  let result = operation()
  let end = 0    // now()
  let duration = end - start
  // info("Operation took " + fromInt(duration) + "ms")  // Would log timing
  result
}

function withRetry<T>(operation: fn() -> Result<T, String>, maxRetries: Int): Result<T, String> uses { time } {
  withRetryHelper(operation, maxRetries, 0)
}

function withRetryHelper<T>(operation: fn() -> Result<T, String>, maxRetries: Int, attempt: Int): Result<T, String> {
  let result = operation()
  match result {
    ok(value) -> ok(value)
    err(_) -> if attempt < maxRetries {
               // sleep(1000 * (attempt + 1))  // Exponential backoff
               withRetryHelper(operation, maxRetries, attempt + 1)
             } else {
               result
             }
  }
}

// Effect isolation
// Functions can be designed to minimize their effect surface area

function pureComputation(data: List<Int>): Int {
  // Pure function - no effects
  foldList(data, 0, fn(acc, x) => acc + x)
}

function effectfulProcessing(data: List<Int>): Int uses { log } {
  // Calls pure function but adds logging effect
  let result = pureComputation(data)
  logAndReturn("Computation result: " + fromInt(result), result)
}

// Effect polymorphism (future feature)
// This would allow functions to be generic over effects
// function mapWithEffects<T, U, Effects>(list: List<T>, f: fn(T) -> U uses Effects): List<U> uses Effects

// Effect documentation
// This module serves as both implementation and documentation for
// the effect system. The actual effect implementations are provided
// by the runtime host environment and injected into the __effects object.
