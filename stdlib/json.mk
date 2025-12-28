language v1.0

module json

// JSON Operations - Boundary type for API serialization
// Note: JSON is only used at API boundaries, never internally

function encode(value: Json): String {
  // Implementation provided by runtime
  // Serializes Json value to JSON string
  "{}"  // Placeholder
}

function decode(json_string: String): Result<Json, String> {
  // Implementation provided by runtime
  // Parses JSON string into Json value
  err("not implemented")  // Placeholder
}

// Constructors for Json values

function string(value: String): Json {
  Json.string(value)
}

function number(value: Int): Json {
  Json.number(value)
}

function bool(value: Bool): Json {
  Json.bool(value)
}

function array(elements: List<Json>): Json {
  Json.array(elements)
}

function object(fields: Map<String, Json>): Json {
  Json.object(fields)
}

function null(): Json {
  Json.null
}

// Accessors for Json values

function get(json: Json, path: String): Result<Json, String> {
  // Implementation provided by runtime
  // Access nested values using dot notation (e.g., "user.name")
  err("not implemented")  // Placeholder
}

function getString(json: Json): Result<String, String> {
  match json {
    Json.string(s) -> ok(s)
    _ -> err("not a string")
  }
}

function getInt(json: Json): Result<Int, String> {
  match json {
    Json.number(n) -> ok(n)
    _ -> err("not a number")
  }
}

function getBool(json: Json): Result<Bool, String> {
  match json {
    Json.bool(b) -> ok(b)
    _ -> err("not a boolean")
  }
}

function getArray(json: Json): Result<List<Json>, String> {
  match json {
    Json.array(a) -> ok(a)
    _ -> err("not an array")
  }
}

function getObject(json: Json): Result<Map<String, Json>, String> {
  match json {
    Json.object(o) -> ok(o)
    _ -> err("not an object")
  }
}

// Validation helpers

function isString(json: Json): Bool {
  match json {
    Json.string(_) -> true
    _ -> false
  }
}

function isNumber(json: Json): Bool {
  match json {
    Json.number(_) -> true
    _ -> false
  }
}

function isBool(json: Json): Bool {
  match json {
    Json.bool(_) -> true
    _ -> false
  }
}

function isArray(json: Json): Bool {
  match json {
    Json.array(_) -> true
    _ -> false
  }
}

function isObject(json: Json): Bool {
  match json {
    Json.object(_) -> true
    _ -> false
  }
}

function isNull(json: Json): Bool {
  match json {
    Json.null -> true
    _ -> false
  }
}

// Array operations

function arrayLength(json_array: Json): Result<Int, String> {
  match json_array {
    Json.array(elements) -> ok(lengthList(elements))
    _ -> err("not an array")
  }
}

function arrayGet(json_array: Json, index: Int): Result<Json, String> {
  match json_array {
    Json.array(elements) -> arrayGetHelper(elements, index, 0)
    _ -> err("not an array")
  }
}

function arrayGetHelper(elements: List<Json>, target_index: Int, current_index: Int): Result<Json, String> {
  if target_index < 0 {
    err("negative index")
  } else {
    match elements {
      cons(h, t) -> if current_index == target_index {
                     ok(h)
                   } else {
                     arrayGetHelper(t, target_index, current_index + 1)
                   }
      nil -> err("index out of bounds")
    }
  }
}

// Object operations

function objectKeys(json_object: Json): Result<List<String>, String> {
  match json_object {
    Json.object(fields) -> ok(keysMap(fields))
    _ -> err("not an object")
  }
}

function objectGet(json_object: Json, key: String): Result<Json, String> {
  match json_object {
    Json.object(fields) -> match getMap(fields, key) {
                             some(value) -> ok(value)
                             none -> err("key not found")
                           }
    _ -> err("not an object")
  }
}

function objectHas(json_object: Json, key: String): Result<Bool, String> {
  match json_object {
    Json.object(fields) -> ok(containsKeyMap(fields, key))
    _ -> err("not an object")
  }
}

// Construction helpers

function fromString(s: String): Json {
  string(s)
}

function fromInt(n: Int): Json {
  number(n)
}

function fromBool(b: Bool): Json {
  bool(b)
}

function fromList(items: List<Json>): Json {
  array(items)
}

function fromMap(fields: Map<String, Json>): Json {
  object(fields)
}

// Pretty printing (for debugging)

function prettyPrint(json: Json): String {
  // Implementation provided by runtime
  // Returns formatted JSON string with indentation
  encode(json)  // Placeholder
}

// Validation

function validateSchema(json: Json, schema: Json): Bool {
  // Implementation provided by runtime
  // Basic JSON schema validation
  true  // Placeholder - would implement JSON Schema validation
}

// Boundary conversion
// These functions help convert between internal types and JSON at API boundaries

function optionToJson<T>(opt: Option<T>, valueToJson: fn(T) -> Json): Json {
  match opt {
    some(v) -> valueToJson(v)
    none    -> null()
  }
}

function jsonToOption<T>(json: Json, jsonToValue: fn(Json) -> Result<T, String>): Result<Option<T>, String> {
  if isNull(json) {
    ok(none)
  } else {
    match jsonToValue(json) {
      ok(value) -> ok(some(value))
      err(msg) -> err(msg)
    }
  }
}

function resultToJson<T, E>(result: Result<T, E>, okToJson: fn(T) -> Json, errToJson: fn(E) -> Json): Json {
  match result {
    ok(v)  -> object(setMap(emptyMap(), "ok", okToJson(v)))
    err(e) -> object(setMap(emptyMap(), "error", errToJson(e)))
  }
}

function listToJson<T>(list: List<T>, itemToJson: fn(T) -> Json): Json {
  array(mapList(list, itemToJson))
}

function jsonToList<T>(json: Json, itemFromJson: fn(Json) -> Result<T, String>): Result<List<T>, String> {
  match json {
    Json.array(elements) -> jsonArrayToList(elements, itemFromJson, nil)
    _ -> err("not an array")
  }
}

function jsonArrayToList<T>(elements: List<Json>, itemFromJson: fn(Json) -> Result<T, String>, acc: List<T>): Result<List<T>, String> {
  match elements {
    cons(h, t) -> match itemFromJson(h) {
                   ok(item) -> jsonArrayToList(t, itemFromJson, cons(item, acc))
                   err(msg) -> err(msg)
                 }
    nil -> ok(reverseList(acc))
  }
}
