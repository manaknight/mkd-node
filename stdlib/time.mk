language v1.0

module time

// Time Operations - Accessing current time and date operations

// Time representation (Unix timestamp in milliseconds)
type Timestamp = Int

// Duration in milliseconds
type Duration = Int

// Basic time functions

function now(): Timestamp uses { time } {
  // Implementation provided by runtime
  // Returns current Unix timestamp in milliseconds
  0  // Placeholder
}

function unixMillis(): Timestamp uses { time } {
  now()  // Alias for now()
}

// Duration constants (in milliseconds)
function millisecond(): Duration {
  1
}

function second(): Duration {
  1000
}

function minute(): Duration {
  60 * 1000
}

function hour(): Duration {
  60 * 60 * 1000
}

function day(): Duration {
  24 * 60 * 60 * 1000
}

function week(): Duration {
  7 * 24 * 60 * 60 * 1000
}

// Duration arithmetic

function addDuration(timestamp: Timestamp, duration: Duration): Timestamp {
  timestamp + duration
}

function subtractDuration(timestamp: Timestamp, duration: Duration): Timestamp {
  timestamp - duration
}

function durationBetween(start: Timestamp, end: Timestamp): Duration {
  end - start
}

// Comparison functions

function isBefore(a: Timestamp, b: Timestamp): Bool {
  a < b
}

function isAfter(a: Timestamp, b: Timestamp): Bool {
  a > b
}

function isEqual(a: Timestamp, b: Timestamp): Bool {
  a == b
}

// Duration operations

function multiplyDuration(duration: Duration, factor: Int): Duration {
  duration * factor
}

function divideDuration(duration: Duration, divisor: Int): Result<Duration, String> {
  if divisor == 0 {
    err("division by zero")
  } else {
    ok(duration / divisor)
  }
}

// Formatting functions

function formatTimestamp(timestamp: Timestamp, format: String): String uses { time } {
  // Implementation provided by runtime
  // Formats timestamp according to format string
  // Supports formats like "YYYY-MM-DD HH:mm:ss", "ISO8601", etc.
  "1970-01-01 00:00:00"  // Placeholder
}

function formatDuration(duration: Duration): String {
  // Formats duration as human-readable string (e.g., "2h 30m 45s")
  formatDurationHelper(duration, "")
}

function formatDurationHelper(remaining: Duration, acc: String): String {
  if remaining >= day() {
    let days = remaining / day()
    let new_acc = acc + fromInt(days) + "d "
    formatDurationHelper(remaining % day(), new_acc)
  } else {
    if remaining >= hour() {
      let hours = remaining / hour()
      let new_acc = acc + fromInt(hours) + "h "
      formatDurationHelper(remaining % hour(), new_acc)
    } else {
      if remaining >= minute() {
        let minutes = remaining / minute()
        let new_acc = acc + fromInt(minutes) + "m "
        formatDurationHelper(remaining % minute(), new_acc)
      } else {
        if remaining >= second() {
          let seconds = remaining / second()
          let new_acc = acc + fromInt(seconds) + "s "
          formatDurationHelper(remaining % second(), new_acc)
        } else {
          if remaining > 0 {
            acc + fromInt(remaining) + "ms"
          } else {
            acc
          }
        }
      }
    }
  }
}

// Parsing functions

function parseTimestamp(date_string: String, format: String): Result<Timestamp, String> uses { time } {
  // Implementation provided by runtime
  // Parses date string according to format
  err("not implemented")  // Placeholder
}

function parseDuration(duration_string: String): Result<Duration, String> {
  // Parses duration strings like "2h 30m 45s", "5000ms", etc.
  parseDurationHelper(duration_string, 0)
}

function parseDurationHelper(input: String, acc: Duration): Result<Duration, String> {
  // This would implement duration string parsing
  // For now, just try to parse as milliseconds
  match toInt(input) {
    ok(ms) -> ok(ms)
    err(_) -> err("invalid duration format")
  }
}

// Sleep function (blocks execution)

function sleep(duration: Duration) uses { time } {
  // Implementation provided by runtime
  // Blocks execution for the specified duration
  ()  // Unit return
}

// Timeout utilities

function withTimeout<T>(operation: fn() -> T, timeout_ms: Duration): Result<T, String> uses { time } {
  let start = now()
  let result = operation()
  let elapsed = now() - start

  if elapsed > timeout_ms {
    err("operation timed out")
  } else {
    ok(result)
  }
}

// Scheduling (conceptual - would need runtime support)

type ScheduledTask<T> {
  id: String
  execute_at: Timestamp
  operation: fn() -> T
  completed: Bool
  result: Option<T>
}

function schedule<T>(operation: fn() -> T, delay: Duration): String uses { time } {
  // Implementation would need runtime support for task scheduling
  // Returns a task ID
  "task-0"  // Placeholder
}

function cancelScheduledTask(task_id: String): Bool uses { time } {
  // Implementation provided by runtime
  // Cancels a scheduled task
  false  // Placeholder
}

function getScheduledTaskResult<T>(task_id: String): Option<T> uses { time } {
  // Implementation provided by runtime
  // Gets result of completed scheduled task
  none  // Placeholder
}

// Time zone handling (conceptual)

type TimeZone {
  name: String
  offset_minutes: Int  // Offset from UTC in minutes
}

function utc(): TimeZone {
  TimeZone("UTC", 0)
}

function localTimeZone(): TimeZone uses { time } {
  // Implementation provided by runtime
  utc()  // Placeholder
}

function convertToTimezone(timestamp: Timestamp, from_tz: TimeZone, to_tz: TimeZone): Timestamp {
  let offset_diff = to_tz.offset_minutes - from_tz.offset_minutes
  addDuration(timestamp, offset_diff * minute())
}

// Stopwatch functionality

type Stopwatch {
  start_time: Option<Timestamp>
  laps: List<Duration>
}

function createStopwatch(): Stopwatch {
  Stopwatch(none, nil)
}

function startStopwatch(stopwatch: Stopwatch): Stopwatch uses { time } {
  Stopwatch(some(now()), stopwatch.laps)
}

function stopStopwatch(stopwatch: Stopwatch): Stopwatch {
  Stopwatch(none, stopwatch.laps)
}

function lapStopwatch(stopwatch: Stopwatch): Result<Stopwatch, String> uses { time } {
  match stopwatch.start_time {
    some(start) -> ok(Stopwatch(some(now()), cons(now() - start, stopwatch.laps)))
    none -> err("stopwatch not started")
  }
}

function getElapsedTime(stopwatch: Stopwatch): Result<Duration, String> uses { time } {
  match stopwatch.start_time {
    some(start) -> ok(now() - start)
    none -> err("stopwatch not started")
  }
}

function getLaps(stopwatch: Stopwatch): List<Duration> {
  stopwatch.laps
}

// Benchmarking helpers

function benchmark<T>(operation: fn() -> T, iterations: Int): BenchmarkResult uses { time } {
  benchmarkHelper(operation, iterations, nil, 0)
}

function benchmarkHelper<T>(operation: fn() -> T, remaining: Int, times: List<Duration>, total_time: Duration): BenchmarkResult uses { time } {
  if remaining <= 0 {
    let avg_time = total_time / max(1, lengthList(times))
    BenchmarkResult(lengthList(times), avg_time, minList(times), maxList(times), total_time)
  } else {
    let start = now()
    let _ = operation()  // Ignore result
    let elapsed = now() - start
    benchmarkHelper(operation, remaining - 1, cons(elapsed, times), total_time + elapsed)
  }
}

type BenchmarkResult {
  iterations: Int
  average_time: Duration
  min_time: Duration
  max_time: Duration
  total_time: Duration
}

// Helper functions for benchmarking

function minList(durations: List<Duration>): Duration {
  match durations {
    cons(h, t) -> foldList(t, h, min)
    nil        -> 0
  }
}

function maxList(durations: List<Duration>): Duration {
  match durations {
    cons(h, t) -> foldList(t, h, max)
    nil        -> 0
  }
}
