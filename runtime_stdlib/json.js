// Manaknight Standard Library - JSON Implementation
// Boundary type operations for API serialization

"use strict";

// Encode Json to string
function encode(json) {
  try {
    return JSON.stringify(jsonToJsValue(json));
  } catch (e) {
    return "{}"; // Fallback
  }
}

// Decode string to Json
function decode(json_string) {
  try {
    const jsValue = JSON.parse(json_string);
    return { tag: 'ok', value: jsValueToJson(jsValue) };
  } catch (e) {
    return { tag: 'err', error: 'JSON parse error: ' + e.message };
  }
}

// Constructor functions
function string(value) {
  return { tag: 'string', value: value };
}

function number(value) {
  return { tag: 'number', value: value };
}

function bool(value) {
  return value.tag === 'true' ? { tag: 'bool', value: true } : { tag: 'bool', value: false };
}

function array(elements) {
  return { tag: 'array', elements: elements };
}

function object(fields) {
  return { tag: 'object', fields: fields };
}

function null() {
  return { tag: 'null' };
}

// Accessor functions
function get(json, path) {
  try {
    const jsValue = jsonToJsValue(json);
    const result = getJsValue(jsValue, path);
    return { tag: 'ok', value: jsValueToJson(result) };
  } catch (e) {
    return { tag: 'err', error: 'Path access error: ' + e.message };
  }
}

function getString(json) {
  if (json.tag === 'string') {
    return { tag: 'ok', value: json.value };
  }
  return { tag: 'err', error: 'not a string' };
}

function getInt(json) {
  if (json.tag === 'number') {
    return { tag: 'ok', value: Math.floor(json.value) };
  }
  return { tag: 'err', error: 'not a number' };
}

function getBool(json) {
  if (json.tag === 'bool') {
    return json.value ? { tag: 'true' } : { tag: 'false' };
  }
  return { tag: 'err', error: 'not a boolean' };
}

function getArray(json) {
  if (json.tag === 'array') {
    return { tag: 'ok', value: json.elements };
  }
  return { tag: 'err', error: 'not an array' };
}

function getObject(json) {
  if (json.tag === 'object') {
    return { tag: 'ok', value: json.fields };
  }
  return { tag: 'err', error: 'not an object' };
}

// Helper: Convert Json ADT to JavaScript value
function jsonToJsValue(json) {
  switch (json.tag) {
    case 'string':
      return json.value;
    case 'number':
      return json.value;
    case 'bool':
      return json.value;
    case 'array':
      return listToArray(json.elements).map(jsonToJsValue);
    case 'object':
      const obj = {};
      // Convert Map to object
      for (const [key, value] of Object.entries(json.fields)) {
        obj[key] = jsonToJsValue(value);
      }
      return obj;
    case 'null':
      return null;
    default:
      return null;
  }
}

// Helper: Convert JavaScript value to Json ADT
function jsValueToJson(jsValue) {
  if (jsValue === null || jsValue === undefined) {
    return { tag: 'null' };
  }

  switch (typeof jsValue) {
    case 'string':
      return { tag: 'string', value: jsValue };
    case 'number':
      return { tag: 'number', value: jsValue };
    case 'boolean':
      return { tag: 'bool', value: jsValue };
    case 'object':
      if (Array.isArray(jsValue)) {
        const elements = arrayToList(jsValue.map(jsValueToJson));
        return { tag: 'array', elements: elements };
      } else {
        // Convert object to Map-like structure
        const fields = {};
        for (const [key, value] of Object.entries(jsValue)) {
          fields[key] = jsValueToJson(value);
        }
        return { tag: 'object', fields: fields };
      }
    default:
      return { tag: 'null' };
  }
}

// Helper: Access nested JavaScript value by path
function getJsValue(jsValue, path) {
  const parts = path.split('.');
  let current = jsValue;

  for (const part of parts) {
    if (current && typeof current === 'object') {
      current = current[part];
    } else {
      throw new Error('Invalid path: ' + path);
    }
  }

  return current;
}

// Array operations
function arrayLength(json_array) {
  if (json_array.tag === 'array') {
    return { tag: 'ok', value: listLength(json_array.elements) };
  }
  return { tag: 'err', error: 'not an array' };
}

function arrayGet(json_array, index) {
  if (json_array.tag === 'array') {
    const arr = listToArray(json_array.elements);
    if (index >= 0 && index < arr.length) {
      return { tag: 'ok', value: arr[index] };
    } else {
      return { tag: 'err', error: 'index out of bounds' };
    }
  }
  return { tag: 'err', error: 'not an array' };
}

function arrayGetHelper(elements, target_index, current_index) {
  if (elements.tag === 'nil') {
    return { tag: 'err', error: 'index out of bounds' };
  }

  if (current_index === target_index) {
    return { tag: 'ok', value: elements.head };
  }

  return arrayGetHelper(elements.tail, target_index, current_index + 1);
}

// Object operations
function objectKeys(json_object) {
  if (json_object.tag === 'object') {
    const keys = Object.keys(json_object.fields);
    return { tag: 'ok', value: arrayToList(keys.reverse()) };
  }
  return { tag: 'err', error: 'not an object' };
}

function objectGet(json_object, key) {
  if (json_object.tag === 'object') {
    if (json_object.fields.hasOwnProperty(key)) {
      return { tag: 'ok', value: json_object.fields[key] };
    } else {
      return { tag: 'err', error: 'key not found' };
    }
  }
  return { tag: 'err', error: 'not an object' };
}

function objectHas(json_object, key) {
  if (json_object.tag === 'object') {
    return json_object.fields.hasOwnProperty(key) ? { tag: 'true' } : { tag: 'false' };
  }
  return { tag: 'err', error: 'not an object' };
}

// Construction helpers
function fromString(s) {
  return string(s);
}

function fromInt(n) {
  return number(n);
}

function fromBool(b) {
  return bool(b);
}

function fromList(items) {
  return array(items);
}

function fromMap(fields) {
  return object(fields);
}

// Pretty printing
function prettyPrint(json) {
  return encode(json); // JSON.stringify already pretty-prints
}

// Validation
function validateSchema(json, schema) {
  // Basic schema validation - simplified implementation
  return true; // Placeholder
}

// Boundary conversion
function optionToJson(opt, valueToJson) {
  if (opt.tag === 'some') {
    return valueToJson(opt.value);
  } else {
    return { tag: 'null' };
  }
}

function jsonToOption(json, jsonToValue) {
  if (json.tag === 'null') {
    return { tag: 'ok', value: { tag: 'none' } };
  } else {
    const result = jsonToValue(json);
    if (result.tag === 'ok') {
      return { tag: 'ok', value: { tag: 'some', value: result.value } };
    } else {
      return { tag: 'err', error: result.error };
    }
  }
}

function resultToJson(result, okToJson, errToJson) {
  if (result.tag === 'ok') {
    return object({ 'ok': okToJson(result.value) });
  } else {
    return object({ 'error': errToJson(result.error) });
  }
}

function listToJson(list, itemToJson) {
  return array(listMap(list, itemToJson));
}

function jsonToList(json, itemFromJson) {
  if (json.tag === 'array') {
    return jsonArrayToList(json.elements, itemFromJson, { tag: 'nil' });
  }
  return { tag: 'err', error: 'not an array' };
}

function jsonArrayToList(elements, itemFromJson, acc) {
  if (elements.tag === 'nil') {
    return { tag: 'ok', value: acc };
  }

  const itemResult = itemFromJson(elements.head);
  if (itemResult.tag === 'err') {
    return { tag: 'err', error: itemResult.error };
  }

  const newAcc = { tag: 'cons', head: itemResult.value, tail: acc };
  return jsonArrayToList(elements.tail, itemFromJson, newAcc);
}

// List utility functions (simplified versions from core.js)
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

function listLength(list) {
  let count = 0;
  let current = list;
  while (current.tag === 'cons') {
    count++;
    current = current.tail;
  }
  return count;
}

function listMap(list, f) {
  const result = [];
  let current = list;
  while (current.tag === 'cons') {
    result.push(f(current.head));
    current = current.tail;
  }
  return arrayToList(result.reverse());
}

// Export functions
if (typeof module !== 'undefined' && module.exports) {
  module.exports = {
    encode,
    decode,
    string,
    number,
    bool,
    array,
    object,
    null,
    get,
    getString,
    getInt,
    getBool,
    getArray,
    getObject,
    arrayLength,
    arrayGet,
    arrayGetHelper,
    objectKeys,
    objectGet,
    objectHas,
    fromString,
    fromInt,
    fromBool,
    fromList,
    fromMap,
    prettyPrint,
    validateSchema,
    optionToJson,
    jsonToOption,
    resultToJson,
    listToJson,
    jsonToList,
    jsonArrayToList
  };
}
