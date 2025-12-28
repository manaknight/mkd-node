// Manaknight Standard Library - HTTP Implementation
// Client and server HTTP operations with effect injection

"use strict";

// HTTP request constructor
function makeHttpRequest(method, url, headers, body) {
  return {
    method: method,
    url: url,
    headers: headers || {},
    body: body
  };
}

// HTTP response constructor
function makeHttpResponse(status_code, headers, body) {
  return {
    status_code: status_code,
    headers: headers || {},
    body: body
  };
}

// HTTP client functions - these will be injected by the runtime
function get(url) {
  return getWithHeaders(url, {});
}

function getWithHeaders(url, headers) {
  const request = makeHttpRequest('GET', url, headers, { tag: 'none' });
  // Runtime will inject __effects.http.get
  if (typeof __effects !== 'undefined' && __effects.http) {
    try {
      const result = __effects.http.get(request);
      return { tag: 'ok', value: result };
    } catch (e) {
      return { tag: 'err', error: { tag: 'network_error', message: e.message } };
    }
  }
  return { tag: 'err', error: { tag: 'network_error', message: 'HTTP not available' } };
}

function post(url, body) {
  return postWithHeaders(url, body, {});
}

function postWithHeaders(url, body, headers) {
  const request = makeHttpRequest('POST', url, headers, { tag: 'some', value: body });
  if (typeof __effects !== 'undefined' && __effects.http) {
    try {
      const result = __effects.http.post(request);
      return { tag: 'ok', value: result };
    } catch (e) {
      return { tag: 'err', error: { tag: 'network_error', message: e.message } };
    }
  }
  return { tag: 'err', error: { tag: 'network_error', message: 'HTTP not available' } };
}

function put(url, body) {
  return putWithHeaders(url, body, {});
}

function putWithHeaders(url, body, headers) {
  const request = makeHttpRequest('PUT', url, headers, { tag: 'some', value: body });
  if (typeof __effects !== 'undefined' && __effects.http) {
    try {
      const result = __effects.http.put(request);
      return { tag: 'ok', value: result };
    } catch (e) {
      return { tag: 'err', error: { tag: 'network_error', message: e.message } };
    }
  }
  return { tag: 'err', error: { tag: 'network_error', message: 'HTTP not available' } };
}

function delete_(url) { // delete is a keyword
  return deleteWithHeaders(url, {});
}

function deleteWithHeaders(url, headers) {
  const request = makeHttpRequest('DELETE', url, headers, { tag: 'none' });
  if (typeof __effects !== 'undefined' && __effects.http) {
    try {
      const result = __effects.http.delete(request);
      return { tag: 'ok', value: result };
    } catch (e) {
      return { tag: 'err', error: { tag: 'network_error', message: e.message } };
    }
  }
  return { tag: 'err', error: { tag: 'network_error', message: 'HTTP not available' } };
}

function patch(url, body) {
  return patchWithHeaders(url, body, {});
}

function patchWithHeaders(url, body, headers) {
  const request = makeHttpRequest('PATCH', url, headers, { tag: 'some', value: body });
  if (typeof __effects !== 'undefined' && __effects.http) {
    try {
      const result = __effects.http.patch(request);
      return { tag: 'ok', value: result };
    } catch (e) {
      return { tag: 'err', error: { tag: 'network_error', message: e.message } };
    }
  }
  return { tag: 'err', error: { tag: 'network_error', message: 'HTTP not available' } };
}

function head(url) {
  return headWithHeaders(url, {});
}

function headWithHeaders(url, headers) {
  const request = makeHttpRequest('HEAD', url, headers, { tag: 'none' });
  if (typeof __effects !== 'undefined' && __effects.http) {
    try {
      const result = __effects.http.head(request);
      return { tag: 'ok', value: result };
    } catch (e) {
      return { tag: 'err', error: { tag: 'network_error', message: e.message } };
    }
  }
  return { tag: 'err', error: { tag: 'network_error', message: 'HTTP not available' } };
}

// Generic request function
function makeRequest(method, url, headers, body) {
  const request = makeHttpRequest(method, url, headers, body);
  if (typeof __effects !== 'undefined' && __effects.http) {
    try {
      const result = __effects.http.request(request);
      return { tag: 'ok', value: result };
    } catch (e) {
      return { tag: 'err', error: { tag: 'network_error', message: e.message } };
    }
  }
  return { tag: 'err', error: { tag: 'network_error', message: 'HTTP not available' } };
}

// Response helpers
function isSuccess(response) {
  return response.status_code >= 200 && response.status_code < 300;
}

function isClientError(response) {
  return response.status_code >= 400 && response.status_code < 500;
}

function isServerError(response) {
  return response.status_code >= 500 && response.status_code < 600;
}

function getHeader(response, name) {
  if (response.headers && response.headers[name]) {
    return { tag: 'some', value: response.headers[name] };
  }
  return { tag: 'none' };
}

function getContentType(response) {
  const header = getHeader(response, 'content-type');
  if (header.tag === 'some') {
    return header.value;
  }
  return 'application/octet-stream';
}

function getBodyString(response) {
  if (response.body && response.body.tag === 'some') {
    return { tag: 'ok', value: response.body.value };
  }
  return { tag: 'err', error: 'no response body' };
}

// Response constructors
function okResponse(status, body) {
  return makeHttpResponse(status, {}, { tag: 'some', value: body });
}

function okResponseWithHeaders(status, body, headers) {
  return makeHttpResponse(status, headers, { tag: 'some', value: body });
}

function errorResponse(status, message) {
  const body = `{"error": "${message}"}`;
  const headers = { 'content-type': 'application/json' };
  return makeHttpResponse(status, headers, { tag: 'some', value: body });
}

function jsonResponse(status, json_body) {
  // This would use the JSON encode function
  const body = JSON.stringify(json_body); // Simplified
  const headers = { 'content-type': 'application/json' };
  return makeHttpResponse(status, headers, { tag: 'some', value: body });
}

function textResponse(status, text) {
  const headers = { 'content-type': 'text/plain' };
  return makeHttpResponse(status, headers, { tag: 'some', value: text });
}

function htmlResponse(status, html) {
  const headers = { 'content-type': 'text/html' };
  return makeHttpResponse(status, headers, { tag: 'some', value: html });
}

// Content-Type constants
function contentTypeJson() {
  return 'application/json';
}

function contentTypeText() {
  return 'text/plain';
}

function contentTypeHtml() {
  return 'text/html';
}

function contentTypeXml() {
  return 'application/xml';
}

// URL encoding/decoding
function urlEncode(s) {
  return encodeURIComponent(s);
}

function urlDecode(s) {
  try {
    return { tag: 'ok', value: decodeURIComponent(s) };
  } catch (e) {
    return { tag: 'err', error: 'URL decode error: ' + e.message };
  }
}

// Query parameter parsing
function getQueryParam(request, name) {
  // This would parse URL query parameters
  // Simplified implementation
  return { tag: 'none' };
}

function getPathParam(request, name) {
  // This would extract path parameters from URL
  // Simplified implementation
  return { tag: 'none' };
}

function getHeader(request, name) {
  if (request.headers && request.headers[name]) {
    return { tag: 'some', value: request.headers[name] };
  }
  return { tag: 'none' };
}

function getBodyString(request) {
  if (request.body && request.body.tag === 'some') {
    return { tag: 'ok', value: request.body.value };
  }
  return { tag: 'err', error: 'no request body' };
}

// Form data handling
function parseFormData(body) {
  try {
    const params = new URLSearchParams(body);
    const result = {};
    for (const [key, value] of params) {
      result[key] = value;
    }
    return { tag: 'ok', value: result };
  } catch (e) {
    return { tag: 'err', error: 'Form data parse error: ' + e.message };
  }
}

function encodeFormData(data) {
  const params = new URLSearchParams();
  for (const [key, value] of Object.entries(data)) {
    params.append(key, value);
  }
  return params.toString();
}

// JSON API helpers
function jsonGet(url) {
  return getWithHeaders(url, { 'accept': 'application/json' })
    .then(response => {
      if (isSuccess(response)) {
        return getBodyString(response)
          .then(body => {
            try {
              const json = JSON.parse(body);
              return { tag: 'ok', value: json };
            } catch (e) {
              return { tag: 'err', error: { tag: 'parse_error', message: 'JSON parse error: ' + e.message } };
            }
          });
      } else {
        return { tag: 'err', error: { tag: 'status_error', code: response.status_code, message: 'HTTP error' } };
      }
    });
}

function jsonPost(url, data) {
  const json_body = JSON.stringify(data);
  const headers = {
    'content-type': 'application/json',
    'accept': 'application/json'
  };

  return postWithHeaders(url, json_body, headers)
    .then(response => {
      if (isSuccess(response)) {
        return getBodyString(response)
          .then(body => {
            try {
              const json = JSON.parse(body);
              return { tag: 'ok', value: json };
            } catch (e) {
              return { tag: 'err', error: { tag: 'parse_error', message: 'JSON parse error: ' + e.message } };
            }
          });
      } else {
        return { tag: 'err', error: { tag: 'status_error', code: response.status_code, message: 'HTTP error' } };
      }
    });
}

// CORS helpers
function corsHeaders(origin) {
  return {
    'access-control-allow-origin': origin,
    'access-control-allow-methods': 'GET, POST, PUT, DELETE, OPTIONS',
    'access-control-allow-headers': 'content-type, authorization'
  };
}

function corsPreflightResponse(origin) {
  return makeHttpResponse(200, corsHeaders(origin), { tag: 'none' });
}

// Rate limiting (conceptual)
function checkRateLimit(key, limit) {
  // This would need runtime support for rate limiting
  return { tag: 'true' }; // Always allow in this implementation
}

// Export functions
if (typeof module !== 'undefined' && module.exports) {
  module.exports = {
    get,
    getWithHeaders,
    post,
    postWithHeaders,
    put,
    putWithHeaders,
    delete: delete_,
    deleteWithHeaders,
    patch,
    patchWithHeaders,
    head,
    headWithHeaders,
    makeRequest,
    isSuccess,
    isClientError,
    isServerError,
    getHeader,
    getContentType,
    getBodyString,
    okResponse,
    okResponseWithHeaders,
    errorResponse,
    jsonResponse,
    textResponse,
    htmlResponse,
    contentTypeJson,
    contentTypeText,
    contentTypeHtml,
    contentTypeXml,
    urlEncode,
    urlDecode,
    getQueryParam,
    getPathParam,
    getHeader,
    getBodyString,
    parseFormData,
    encodeFormData,
    jsonGet,
    jsonPost,
    corsHeaders,
    corsPreflightResponse,
    checkRateLimit
  };
}
