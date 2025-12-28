language v1.0

module log

// Logging Operations - Structured logging with different levels

// Log Level
type LogLevel {
  | DEBUG
  | INFO
  | WARN
  | ERROR
}

// Log Entry (for structured logging)
type LogEntry {
  level: LogLevel
  message: String
  timestamp: Int  // Unix timestamp in milliseconds
  fields: Map<String, String>  // Additional structured fields
}

// Basic Logging Functions

function debug(message: String) uses { log } {
  logMessage(LogLevel.DEBUG, message, emptyMap())
}

function info(message: String) uses { log } {
  logMessage(LogLevel.INFO, message, emptyMap())
}

function warn(message: String) uses { log } {
  logMessage(LogLevel.WARN, message, emptyMap())
}

function error(message: String) uses { log } {
  logMessage(LogLevel.ERROR, message, emptyMap())
}

// Structured Logging Functions

function debugWithFields(message: String, fields: Map<String, String>) uses { log } {
  logMessage(LogLevel.DEBUG, message, fields)
}

function infoWithFields(message: String, fields: Map<String, String>) uses { log } {
  logMessage(LogLevel.INFO, message, fields)
}

function warnWithFields(message: String, fields: Map<String, String>) uses { log } {
  logMessage(LogLevel.WARN, message, fields)
}

function errorWithFields(message: String, fields: Map<String, String>) uses { log } {
  logMessage(LogLevel.ERROR, message, fields)
}

// Generic logging function (implementation provided by runtime)
function logMessage(level: LogLevel, message: String, fields: Map<String, String>) uses { log } {
  // Implementation provided by runtime
  // Creates a LogEntry and sends it to the logging system
  ()  // Unit return
}

// Context-aware logging

function withContext(context: Map<String, String>, operation: fn() -> ()) uses { log } {
  // Implementation would add context to all log messages within operation
  // For now, just execute the operation
  operation()
}

function withRequestId(request_id: String, operation: fn() -> ()) uses { log } {
  withContext(setMap(emptyMap(), "request_id", request_id), operation)
}

function withUserId(user_id: String, operation: fn() -> ()) uses { log } {
  withContext(setMap(emptyMap(), "user_id", user_id), operation)
}

// Performance logging

function logDuration<T>(operation_name: String, operation: fn() -> T): T uses { log, time } {
  let start = 0  // now()
  let result = operation()
  let end = 0    // now()
  let duration = end - start

  infoWithFields(
    operation_name + " completed",
    setMap(emptyMap(), "duration_ms", fromInt(duration))
  )

  result
}

// Error logging helpers

function logError(error: String) uses { log } {
  error(error)
}

function logErrorWithContext(error: String, context: Map<String, String>) uses { log } {
  errorWithFields(error, context)
}

function logAndReturn<T>(level: LogLevel, message: String, value: T): T uses { log } {
  logMessage(level, message, emptyMap())
  value
}

function logAndThrow<T>(level: LogLevel, message: String, error: String): T uses { log } {
  logMessage(level, message, emptyMap())
  // In a language with exceptions, this would throw
  // For now, we'll just log and return a default value
  match level {
    LogLevel.ERROR -> error("Fatal error: " + error)
    _ -> warn("Non-fatal error: " + error)
  }
  // This would need to be adjusted based on the return type T
  // For now, we'll assume T is Result<String, String> or similar
  err("logged error: " + error)
}

// Log formatting

function formatMessage(template: String, args: Map<String, String>): String {
  // Simple template replacement: {key} -> value
  formatMessageHelper(template, args)
}

function formatMessageHelper(template: String, args: Map<String, String>): String {
  // This would implement template replacement
  // For now, return the template as-is
  template
}

// Level checking (for conditional logging)

function isLevelEnabled(level: LogLevel): Bool uses { log } {
  // Implementation provided by runtime
  // Checks if the given log level is enabled
  true  // Placeholder - assume all levels enabled
}

function ifDebugEnabled(operation: fn() -> ()) uses { log } {
  if isLevelEnabled(LogLevel.DEBUG) {
    operation()
  } else {
    ()
  }
}

function ifInfoEnabled(operation: fn() -> ()) uses { log } {
  if isLevelEnabled(LogLevel.INFO) {
    operation()
  } else {
    ()
  }
}

// Batch logging

function logMultiple(entries: List<LogEntry>) uses { log } {
  // Implementation provided by runtime
  // Logs multiple entries efficiently
  ()  // Placeholder
}

// Log entry constructors

function debugEntry(message: String): LogEntry {
  LogEntry(LogLevel.DEBUG, message, 0, emptyMap())
}

function infoEntry(message: String): LogEntry {
  LogEntry(LogLevel.INFO, message, 0, emptyMap())
}

function warnEntry(message: String): LogEntry {
  LogEntry(LogLevel.WARN, message, 0, emptyMap())
}

function errorEntry(message: String): LogEntry {
  LogEntry(LogLevel.ERROR, message, 0, emptyMap())
}

function entryWithFields(level: LogLevel, message: String, fields: Map<String, String>): LogEntry {
  LogEntry(level, message, 0, fields)
}

// Redaction helpers (for sensitive data)

function redactString(s: String): String {
  // Implementation provided by runtime
  // Returns a redacted version of the string (e.g., "***")
  "***"
}

function redactMap(fields: Map<String, String>, sensitive_keys: List<String>): Map<String, String> {
  // Redacts values for sensitive keys
  foldList(sensitive_keys, fields, fn(acc, key) =>
    if containsKeyMap(acc, key) {
      setMap(acc, key, redactString(getMap(acc, key)))
    } else {
      acc
    }
  )
}

// Common sensitive keys
function defaultSensitiveKeys(): List<String> {
  cons("password", cons("token", cons("secret", cons("key", nil))))
}

// Safe logging functions

function safeLogUserData(user_data: Map<String, String>) uses { log } {
  let safe_data = redactMap(user_data, defaultSensitiveKeys())
  infoWithFields("User data logged", safe_data)
}

function safeLogRequest(request_data: Map<String, String>) uses { log } {
  let sensitive_keys = cons("authorization", cons("cookie", defaultSensitiveKeys()))
  let safe_data = redactMap(request_data, sensitive_keys)
  debugWithFields("Request data logged", safe_data)
}

// Log aggregation (conceptual - would need runtime support)

type LogStats {
  total_entries: Int
  by_level: Map<String, Int>  // "DEBUG" -> count, etc.
  time_range: Option<(Int, Int)>  // (start, end) timestamps
}

function getLogStats(since: Option<Int>): LogStats uses { log } {
  // Implementation would need runtime support for log aggregation
  LogStats(0, emptyMap(), none)  // Placeholder
}

// Export functions for external consumption
// These would be used by monitoring/logging systems

function exportLogs(since: Int, level: Option<LogLevel>): List<LogEntry> uses { log } {
  // Implementation provided by runtime
  // Returns log entries for export/processing
  nil  // Placeholder
}
