language v1.0

module http

// HTTP Operations - Effect-based HTTP client and server operations

// HTTP Method types
type HttpMethod {
  | GET
  | POST
  | PUT
  | DELETE
  | PATCH
  | HEAD
  | OPTIONS
}

// HTTP Headers
type HttpHeaders = Map<String, String>

// HTTP Request
type HttpRequest {
  method: HttpMethod
  url: String
  headers: HttpHeaders
  body: Option<String>
}

// HTTP Response
type HttpResponse {
  status_code: Int
  headers: HttpHeaders
  body: Option<String>
}

// HTTP Error types
type HttpError {
  | network_error(message: String)
  | timeout_error
  | status_error(code: Int, message: String)
  | parse_error(message: String)
}

// HTTP Client Functions

function get(url: String): Result<HttpResponse, HttpError> uses { http } {
  getWithHeaders(url, emptyMap())
}

function getWithHeaders(url: String, headers: HttpHeaders): Result<HttpResponse, HttpError> uses { http } {
  makeRequest(HttpMethod.GET, url, headers, none)
}

function post(url: String, body: String): Result<HttpResponse, HttpError> uses { http } {
  postWithHeaders(url, body, emptyMap())
}

function postWithHeaders(url: String, body: String, headers: HttpHeaders): Result<HttpResponse, HttpError> uses { http } {
  makeRequest(HttpMethod.POST, url, headers, some(body))
}

function put(url: String, body: String): Result<HttpResponse, HttpError> uses { http } {
  putWithHeaders(url, body, emptyMap())
}

function putWithHeaders(url: String, body: String, headers: HttpHeaders): Result<HttpResponse, HttpError> uses { http } {
  makeRequest(HttpMethod.PUT, url, headers, some(body))
}

function delete(url: String): Result<HttpResponse, HttpError> uses { http } {
  deleteWithHeaders(url, emptyMap())
}

function deleteWithHeaders(url: String, headers: HttpHeaders): Result<HttpResponse, HttpError> uses { http } {
  makeRequest(HttpMethod.DELETE, url, headers, none)
}

function patch(url: String, body: String): Result<HttpResponse, HttpError> uses { http } {
  patchWithHeaders(url, body, emptyMap())
}

function patchWithHeaders(url: String, body: String, headers: HttpHeaders): Result<HttpResponse, HttpError> uses { http } {
  makeRequest(HttpMethod.PATCH, url, headers, some(body))
}

function head(url: String): Result<HttpResponse, HttpError> uses { http } {
  headWithHeaders(url, emptyMap())
}

function headWithHeaders(url: String, headers: HttpHeaders): Result<HttpResponse, HttpError> uses { http } {
  makeRequest(HttpMethod.HEAD, url, headers, none)
}

// Generic request function (implementation provided by runtime)
function makeRequest(method: HttpMethod, url: String, headers: HttpHeaders, body: Option<String>): Result<HttpResponse, HttpError> uses { http } {
  // Implementation provided by runtime
  err(HttpError.network_error("not implemented"))  // Placeholder
}

// HTTP Response Helpers

function isSuccess(response: HttpResponse): Bool {
  response.status_code >= 200 && response.status_code < 300
}

function isClientError(response: HttpResponse): Bool {
  response.status_code >= 400 && response.status_code < 500
}

function isServerError(response: HttpResponse): Bool {
  response.status_code >= 500 && response.status_code < 600
}

function getHeader(response: HttpResponse, name: String): Option<String> {
  getMap(response.headers, name)
}

function getContentType(response: HttpResponse): String {
  match getHeader(response, "content-type") {
    some(ct) -> ct
    none     -> "application/octet-stream"
  }
}

function getBodyString(response: HttpResponse): Result<String, String> {
  match response.body {
    some(body) -> ok(body)
    none       -> err("no response body")
  }
}

// HTTP Server Functions (for API handlers)

function getQueryParam(request: HttpRequest, name: String): Option<String> {
  // Implementation provided by runtime
  // Extract query parameter from URL
  none  // Placeholder
}

function getPathParam(request: HttpRequest, name: String): Option<String> {
  // Implementation provided by runtime
  // Extract path parameter from URL
  none  // Placeholder
}

function getHeader(request: HttpRequest, name: String): Option<String> {
  getMap(request.headers, name)
}

function getBodyString(request: HttpRequest): Result<String, String> {
  match request.body {
    some(body) -> ok(body)
    none       -> err("no request body")
  }
}

// Response Constructors

function okResponse(status: Int, body: String): HttpResponse {
  HttpResponse(status, emptyMap(), some(body))
}

function okResponseWithHeaders(status: Int, body: String, headers: HttpHeaders): HttpResponse {
  HttpResponse(status, headers, some(body))
}

function errorResponse(status: Int, message: String): HttpResponse {
  let body = "{\"error\": \"" + message + "\"}"
  let headers = setMap(emptyMap(), "content-type", "application/json")
  HttpResponse(status, headers, some(body))
}

function jsonResponse(status: Int, json_body: Json): HttpResponse uses { json } {
  let body = encode(json_body)
  let headers = setMap(emptyMap(), "content-type", "application/json")
  HttpResponse(status, headers, some(body))
}

function textResponse(status: Int, text: String): HttpResponse {
  let headers = setMap(emptyMap(), "content-type", "text/plain")
  HttpResponse(status, headers, some(text))
}

function htmlResponse(status: Int, html: String): HttpResponse {
  let headers = setMap(emptyMap(), "content-type", "text/html")
  HttpResponse(status, headers, some(html))
}

// Content-Type Constants
function contentTypeJson(): String {
  "application/json"
}

function contentTypeText(): String {
  "text/plain"
}

function contentTypeHtml(): String {
  "text/html"
}

function contentTypeXml(): String {
  "application/xml"
}

// URL Encoding/Decoding

function urlEncode(s: String): String {
  // Implementation provided by runtime
  // URL-encode a string
  s  // Placeholder
}

function urlDecode(s: String): Result<String, String> {
  // Implementation provided by runtime
  // URL-decode a string
  ok(s)  // Placeholder
}

// Form Data Handling

function parseFormData(body: String): Result<Map<String, String>, String> {
  // Implementation provided by runtime
  // Parse application/x-www-form-urlencoded data
  ok(emptyMap())  // Placeholder
}

function encodeFormData(data: Map<String, String>): String {
  // Implementation provided by runtime
  // Encode data as application/x-www-form-urlencoded
  ""  // Placeholder
}

// JSON API Helpers

function jsonGet(url: String): Result<Json, HttpError> uses { http, json } {
  match getWithHeaders(url, setMap(emptyMap(), "accept", contentTypeJson())) {
    ok(response) -> if isSuccess(response) {
                     match getBodyString(response) {
                       ok(body) -> match decode(body) {
                                    ok(json_val) -> ok(json_val)
                                    err(msg) -> err(HttpError.parse_error("JSON parse error: " + msg))
                                  }
                       err(msg) -> err(HttpError.parse_error("No response body: " + msg))
                     }
                   } else {
                     err(HttpError.status_error(response.status_code, "HTTP error"))
                   }
    err(http_err) -> err(http_err)
  }
}

function jsonPost(url: String, data: Json): Result<Json, HttpError> uses { http, json } {
  let json_body = encode(data)
  let headers = mergeMaps(
    setMap(emptyMap(), "content-type", contentTypeJson()),
    setMap(emptyMap(), "accept", contentTypeJson())
  )

  match postWithHeaders(url, json_body, headers) {
    ok(response) -> if isSuccess(response) {
                     match getBodyString(response) {
                       ok(body) -> match decode(body) {
                                    ok(json_val) -> ok(json_val)
                                    err(msg) -> err(HttpError.parse_error("JSON parse error: " + msg))
                                  }
                       err(msg) -> err(HttpError.parse_error("No response body: " + msg))
                     }
                   } else {
                     err(HttpError.status_error(response.status_code, "HTTP error"))
                   }
    err(http_err) -> err(http_err)
  }
}

// CORS Helpers

function corsHeaders(origin: String): HttpHeaders {
  mergeMaps(
    setMap(emptyMap(), "access-control-allow-origin", origin),
    mergeMaps(
      setMap(emptyMap(), "access-control-allow-methods", "GET, POST, PUT, DELETE, OPTIONS"),
      setMap(emptyMap(), "access-control-allow-headers", "content-type, authorization")
    )
  )
}

function corsPreflightResponse(origin: String): HttpResponse {
  HttpResponse(200, corsHeaders(origin), none)
}

// Rate Limiting Helpers (conceptual - would need runtime support)

type RateLimit {
  requests_per_minute: Int
  current_count: Int
  window_start: Int
}

function checkRateLimit(key: String, limit: Int): Bool uses { http } {
  // Implementation would need runtime support for rate limiting
  // This is just a conceptual example
  true  // Placeholder - always allow
}
