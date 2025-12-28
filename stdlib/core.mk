language v1.0

module core

// Core Types - The foundation of the standard library

// Option<T> - Represents optional values
type Option<T> {
  | some(value: T)
  | none
}

// Result<T, E> - Represents success/failure operations
type Result<T, E> {
  | ok(value: T)
  | err(error: E)
}

// Bool - Boolean type (though defined as ADT for consistency)
type Bool {
  | true
  | false
}

// List<T> - Immutable linked list
type List<T> {
  | cons(head: T, tail: List<T>)
  | nil
}

// Map<K, V> - Immutable key-value mapping
// Note: Keys must be structurally comparable
type Map<K, V>

// Json - JSON boundary type for API serialization
type Json {
  | string(value: String)
  | number(value: Int)  // Simplified - could be Float in future
  | bool(value: Bool)
  | array(elements: List<Json>)
  | object(fields: Map<String, Json>)
  | null
}

// Core Functions - Pure operations available everywhere

// Identity function
function identity<T>(x: T): T {
  x
}

// Equality - structural equality for all types
function equals<T>(a: T, b: T): Bool {
  // Implementation provided by runtime
  // This is a primitive that gets special handling
  true  // Placeholder - runtime implements actual equality
}

// Hash function for use in maps/sets
function hash<T>(value: T): Int {
  // Implementation provided by runtime
  0  // Placeholder
}

// Compose two functions
function compose<A, B, C>(f: fn(B) -> C, g: fn(A) -> B): fn(A) -> C {
  fn(x: A) => f(g(x))
}

// Pipe value through function (syntactic sugar, but defined here for completeness)
function pipe<A, B>(value: A, f: fn(A) -> B): B {
  f(value)
}

// Option Operations

function mapOption<T, U>(option: Option<T>, f: fn(T) -> U): Option<U> {
  match option {
    some(v) -> some(f(v))
    none    -> none
  }
}

function flatMapOption<T, U>(option: Option<T>, f: fn(T) -> Option<U>): Option<U> {
  match option {
    some(v) -> f(v)
    none    -> none
  }
}

function unwrapOr<T>(option: Option<T>, default: T): T {
  match option {
    some(v) -> v
    none    -> default
  }
}

function isSome<T>(option: Option<T>): Bool {
  match option {
    some(_) -> true
    none    -> false
  }
}

function isNone<T>(option: Option<T>): Bool {
  match option {
    some(_) -> false
    none    -> true
  }
}

// Result Operations

function mapResult<T, U, E>(result: Result<T, E>, f: fn(T) -> U): Result<U, E> {
  match result {
    ok(v)  -> ok(f(v))
    err(e) -> err(e)
  }
}

function flatMapResult<T, U, E>(result: Result<T, E>, f: fn(T) -> Result<U, E>): Result<U, E> {
  match result {
    ok(v)  -> f(v)
    err(e) -> err(e)
  }
}

function mapError<T, E, F>(result: Result<T, E>, f: fn(E) -> F): Result<T, F> {
  match result {
    ok(v)  -> ok(v)
    err(e) -> err(f(e))
  }
}

function unwrapOrResult<T, E>(result: Result<T, E>, default: T): T {
  match result {
    ok(v)  -> v
    err(_) -> default
  }
}

// Bool Operations (though these could be built-in)

function not(b: Bool): Bool {
  match b {
    true  -> false
    false -> true
  }
}

function and(a: Bool, b: Bool): Bool {
  match a {
    true  -> b
    false -> false
  }
}

function or(a: Bool, b: Bool): Bool {
  match a {
    true  -> true
    false -> b
  }
}

// List Operations

function mapList<T, U>(list: List<T>, f: fn(T) -> U): List<U> {
  match list {
    cons(h, t) -> cons(f(h), mapList(t, f))
    nil        -> nil
  }
}

function filterList<T>(list: List<T>, pred: fn(T) -> Bool): List<T> {
  match list {
    cons(h, t) -> match pred(h) {
                   true  -> cons(h, filterList(t, pred))
                   false -> filterList(t, pred)
                 }
    nil        -> nil
  }
}

function foldList<T, U>(list: List<T>, init: U, f: fn(U, T) -> U): U {
  match list {
    cons(h, t) -> foldList(t, f(init, h), f)
    nil        -> init
  }
}

function foldRightList<T, U>(list: List<T>, init: U, f: fn(T, U) -> U): U {
  match list {
    cons(h, t) -> f(h, foldRightList(t, init, f))
    nil        -> init
  }
}

function lengthList<T>(list: List<T>): Int {
  foldList(list, 0, fn(acc, _) => acc + 1)
}

function reverseList<T>(list: List<T>): List<T> {
  foldList(list, nil, fn(acc, x) => cons(x, acc))
}

function takeList<T>(list: List<T>, n: Int): List<T> {
  if n <= 0 {
    nil
  } else {
    match list {
      cons(h, t) -> cons(h, takeList(t, n - 1))
      nil        -> nil
    }
  }
}

function dropList<T>(list: List<T>, n: Int): List<T> {
  if n <= 0 {
    list
  } else {
    match list {
      cons(_, t) -> dropList(t, n - 1)
      nil        -> nil
    }
  }
}

function findList<T>(list: List<T>, pred: fn(T) -> Bool): Option<T> {
  match list {
    cons(h, t) -> match pred(h) {
                   true  -> some(h)
                   false -> findList(t, pred)
                 }
    nil        -> none
  }
}

function allList<T>(list: List<T>, pred: fn(T) -> Bool): Bool {
  match list {
    cons(h, t) -> match pred(h) {
                   true  -> allList(t, pred)
                   false -> false
                 }
    nil        -> true
  }
}

function anyList<T>(list: List<T>, pred: fn(T) -> Bool): Bool {
  match list {
    cons(h, t) -> match pred(h) {
                   true  -> true
                   false -> anyList(t, pred)
                 }
    nil        -> false
  }
}

// Map Operations

function emptyMap<K, V>(): Map<K, V> {
  // Implementation provided by runtime
  emptyMap  // Placeholder
}

function getMap<K, V>(map: Map<K, V>, key: K): Option<V> {
  // Implementation provided by runtime
  none  // Placeholder
}

function setMap<K, V>(map: Map<K, V>, key: K, value: V): Map<K, V> {
  // Implementation provided by runtime - returns new immutable map
  map  // Placeholder
}

function removeMap<K, V>(map: Map<K, V>, key: K): Map<K, V> {
  // Implementation provided by runtime - returns new immutable map
  map  // Placeholder
}

function containsKeyMap<K, V>(map: Map<K, V>, key: K): Bool {
  // Implementation provided by runtime
  false  // Placeholder
}

function keysMap<K, V>(map: Map<K, V>): List<K> {
  // Implementation provided by runtime
  nil  // Placeholder
}

function valuesMap<K, V>(map: Map<K, V>): List<V> {
  // Implementation provided by runtime
  nil  // Placeholder
}

function mergeMaps<K, V>(a: Map<K, V>, b: Map<K, V>): Map<K, V> {
  // Implementation provided by runtime - merges with b taking precedence
  a  // Placeholder
}
