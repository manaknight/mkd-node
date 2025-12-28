// Manaknight Standard Library - Core Implementation
// This file contains the runtime implementations of core Manaknight operations

"use strict";

// Core identity function
function identity(x) {
  return x;
}

// Structural equality - deep equality for all Manaknight types
function equals(a, b) {
  if (a === b) return true;

  // Handle ADT objects (tagged unions)
  if (a && b && typeof a === 'object' && typeof b === 'object') {
    if (a.tag !== b.tag) return false;

    // For ADTs, compare based on tag and values
    switch (a.tag) {
      case 'some':
        return equals(a.value, b.value);
      case 'none':
        return true;
      case 'ok':
        return equals(a.value, b.value);
      case 'err':
        return equals(a.error, b.error);
      case 'cons':
        return equals(a.head, b.head) && equals(a.tail, b.tail);
      case 'nil':
        return true;
      default:
        // For other objects, do structural equality
        return structuralEquals(a, b);
    }
  }

  return false;
}

// Helper for structural equality of plain objects
function structuralEquals(a, b) {
  const keysA = Object.keys(a).sort();
  const keysB = Object.keys(b).sort();

  if (keysA.length !== keysB.length) return false;

  for (let i = 0; i < keysA.length; i++) {
    if (keysA[i] !== keysB[i]) return false;
    if (!equals(a[keysA[i]], b[keysB[i]])) return false;
  }

  return true;
}

// Hash function for structural hashing
function hash(value) {
  // Simple structural hash - in production this would be more sophisticated
  if (value === null || value === undefined) return 0;
  if (typeof value === 'boolean') return value ? 1 : 0;
  if (typeof value === 'number') return value | 0; // Simple truncation
  if (typeof value === 'string') return djb2Hash(value);
  if (typeof value === 'object') {
    if (value.tag) {
      // ADT object
      switch (value.tag) {
        case 'some': return 31 + hash(value.value);
        case 'none': return 37;
        case 'ok': return 41 + hash(value.value);
        case 'err': return 43 + hash(value.error);
        case 'cons': return 47 + hash(value.head) + hash(value.tail);
        case 'nil': return 53;
        default: return structuralHash(value);
      }
    } else {
      return structuralHash(value);
    }
  }
  return 0;
}

// DJB2 string hash
function djb2Hash(str) {
  let hash = 5381;
  for (let i = 0; i < str.length; i++) {
    hash = ((hash << 5) + hash) + str.charCodeAt(i);
  }
  return hash;
}

// Structural hash for objects
function structuralHash(obj) {
  let h = 0;
  const keys = Object.keys(obj).sort();
  for (const key of keys) {
    h = ((h << 5) - h) + djb2Hash(key);
    h = ((h << 5) - h) + hash(obj[key]);
  }
  return h;
}

// Function composition
function compose(f, g) {
  return function (x) {
    return f(g(x));
  };
}

// Pipe function (syntactic sugar for composition)
function pipe(value, f) {
  return f(value);
}

// Boolean operations (though implemented as ADT in Manaknight)
function not(b) {
  return b.tag === 'true' ? { tag: 'false' } : { tag: 'true' };
}

function and(a, b) {
  return a.tag === 'true' ? b : { tag: 'false' };
}

function or(a, b) {
  return a.tag === 'true' ? { tag: 'true' } : b;
}

// Option operations
function mapOption(option, f) {
  if (option.tag === 'some') {
    return { tag: 'some', value: f(option.value) };
  }
  return { tag: 'none' };
}

function flatMapOption(option, f) {
  if (option.tag === 'some') {
    return f(option.value);
  }
  return { tag: 'none' };
}

function unwrapOr(option, defaultValue) {
  return option.tag === 'some' ? option.value : defaultValue;
}

function isSome(option) {
  return option.tag === 'some';
}

function isNone(option) {
  return option.tag === 'none';
}

// Result operations
function mapResult(result, f) {
  if (result.tag === 'ok') {
    return { tag: 'ok', value: f(result.value) };
  }
  return result; // err case
}

function flatMapResult(result, f) {
  if (result.tag === 'ok') {
    return f(result.value);
  }
  return result; // err case
}

function mapError(result, f) {
  if (result.tag === 'err') {
    return { tag: 'err', error: f(result.error) };
  }
  return result; // ok case
}

function unwrapOrResult(result, defaultValue) {
  return result.tag === 'ok' ? result.value : defaultValue;
}

// List operations - implemented iteratively to avoid stack overflow
function mapList(list, f) {
  const result = [];
  let current = list;
  while (current.tag === 'cons') {
    result.push(f(current.head));
    current = current.tail;
  }
  // Reverse and convert back to cons/nil structure
  return arrayToList(result.reverse());
}

function filterList(list, pred) {
  const result = [];
  let current = list;
  while (current.tag === 'cons') {
    if (pred(current.head).tag === 'true') {
      result.push(current.head);
    }
    current = current.tail;
  }
  return arrayToList(result.reverse());
}

function foldList(list, init, f) {
  let acc = init;
  let current = list;
  while (current.tag === 'cons') {
    acc = f(acc, current.head);
    current = current.tail;
  }
  return acc;
}

function foldRightList(list, init, f) {
  const arr = listToArray(list);
  let acc = init;
  for (let i = arr.length - 1; i >= 0; i--) {
    acc = f(arr[i], acc);
  }
  return acc;
}

function lengthList(list) {
  let count = 0;
  let current = list;
  while (current.tag === 'cons') {
    count++;
    current = current.tail;
  }
  return count;
}

function reverseList(list) {
  return arrayToList(listToArray(list).reverse());
}

function takeList(list, n) {
  if (n <= 0) return { tag: 'nil' };
  const result = [];
  let current = list;
  let count = 0;
  while (current.tag === 'cons' && count < n) {
    result.push(current.head);
    current = current.tail;
    count++;
  }
  return arrayToList(result.reverse());
}

function dropList(list, n) {
  let current = list;
  for (let i = 0; i < n && current.tag === 'cons'; i++) {
    current = current.tail;
  }
  return current;
}

function findList(list, pred) {
  let current = list;
  while (current.tag === 'cons') {
    if (pred(current.head).tag === 'true') {
      return { tag: 'some', value: current.head };
    }
    current = current.tail;
  }
  return { tag: 'none' };
}

function allList(list, pred) {
  let current = list;
  while (current.tag === 'cons') {
    if (pred(current.head).tag === 'false') {
      return { tag: 'false' };
    }
    current = current.tail;
  }
  return { tag: 'true' };
}

function anyList(list, pred) {
  let current = list;
  while (current.tag === 'cons') {
    if (pred(current.head).tag === 'true') {
      return { tag: 'true' };
    }
    current = current.tail;
  }
  return { tag: 'false' };
}

// List utility functions
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

// Map operations - using JavaScript objects for string keys
function emptyMap() {
  return {};
}

function getMap(map, key) {
  const keyHash = hash(key);
  if (map.hasOwnProperty(keyHash)) {
    return { tag: 'some', value: map[keyHash] };
  }
  return { tag: 'none' };
}

function setMap(map, key, value) {
  const newMap = Object.assign({}, map);
  newMap[hash(key)] = value;
  return newMap;
}

function removeMap(map, key) {
  const newMap = Object.assign({}, map);
  delete newMap[hash(key)];
  return newMap;
}

function containsKeyMap(map, key) {
  return map.hasOwnProperty(hash(key));
}

function keysMap(map) {
  const keys = [];
  for (const keyHash in map) {
    if (map.hasOwnProperty(keyHash)) {
      // Note: We can't reverse the hash, so we return hash values
      // In a real implementation, we'd store key-value pairs
      keys.push(parseInt(keyHash));
    }
  }
  return arrayToList(keys.reverse());
}

function valuesMap(map) {
  const values = [];
  for (const keyHash in map) {
    if (map.hasOwnProperty(keyHash)) {
      values.push(map[keyHash]);
    }
  }
  return arrayToList(values.reverse());
}

function mergeMaps(a, b) {
  const result = Object.assign({}, a);
  for (const keyHash in b) {
    if (b.hasOwnProperty(keyHash)) {
      result[keyHash] = b[keyHash];
    }
  }
  return result;
}

// Export the functions (in a module system this would be different)
if (typeof module !== 'undefined' && module.exports) {
  module.exports = {
    identity,
    equals,
    hash,
    compose,
    pipe,
    not,
    and,
    or,
    mapOption,
    flatMapOption,
    unwrapOr,
    isSome,
    isNone,
    mapResult,
    flatMapResult,
    mapError,
    unwrapOrResult,
    mapList,
    filterList,
    foldList,
    foldRightList,
    lengthList,
    reverseList,
    takeList,
    dropList,
    findList,
    allList,
    anyList,
    emptyMap,
    getMap,
    setMap,
    removeMap,
    containsKeyMap,
    keysMap,
    valuesMap,
    mergeMaps
  };
}
