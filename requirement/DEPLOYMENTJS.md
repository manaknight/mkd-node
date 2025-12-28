## 1️⃣ What “PHP-style server” Actually Means

When people say *“PHP-style”*, they usually mean:

| PHP-FPM property            | Description                    |
| --------------------------- | ------------------------------ |
| Per-request execution       | Each request runs in isolation |
| No shared application state | State dies at request end      |
| Crash containment           | One request can’t kill others  |
| Stateless workers           | Easy horizontal scaling        |
| Simple mental model         | Request in → response out      |

This is **not** classic Apache `mod_php` anymore — it’s **PHP-FPM**.

---

## 2️⃣ Node vs PHP-FPM vs Manaknight (Mental Model)

### Node.js

```text
One process
One event loop
Shared heap
Shared globals
Async everywhere
```

### PHP-FPM

```text
Many workers
One request per worker
Process isolation
Stateless by default
```

### Manaknight

```text
One server process
One VM per request
Memory + CPU capped
No shared state
Deterministic execution
```

👉 **Manaknight is conceptually PHP-FPM, but without OS process overhead and without footguns.**

---

## 3️⃣ Yes — You Can Run Manaknight “PHP-Style”

There are **two valid deployment styles**, and both work.

---

## Option A (Recommended): Built-in Manaknight Runtime (Best)

This is the *native* model.

```text
[ Load Balancer ]
        ↓
[ Manaknight Runtime ]
        ↓
[ VM per request ]
```

* One native process
* Many isolated VMs
* Fast startup
* Tight resource control
* No process spawning cost

This gives you **PHP-FPM semantics**, but:

* lower memory
* better determinism
* stronger isolation

---

## Option B: External Process-per-Request (Pure PHP Emulation)

You *can* do this too, though it’s usually unnecessary.

```text
[ NGINX ]
   ↓ FastCGI
[ Spawn manaknight-vm ]
   ↓
[ Execute bytecode ]
   ↓
[ Exit ]
```

This is **literally PHP-FPM**, but with Manaknight bytecode.

### Downsides

* OS process spawn cost
* Slower
* More moving parts

### Upside

* Extreme isolation
* Easy mental model
* Familiar to PHP ops teams

---

## 4️⃣ Why Manaknight Is Actually *Better* Than PHP

Let’s be blunt.

### PHP-FPM problems

| PHP issue               | Manaknight           |
| ----------------------- | -------------------- |
| Global variables        | ❌ impossible         |
| Hidden side effects     | ❌ impossible         |
| Runtime type errors     | ❌ compile-time       |
| Accidental shared state | ❌ impossible         |
| Unbounded memory        | ❌ capped             |
| Weak determinism        | ❌ strong determinism |

PHP relies on:

> “Developers behaving.”

Manaknight relies on:

> “The compiler enforcing.”

---

## 5️⃣ “Request Script” Analogy (PHP Mindset)

Think of each API as a **typed PHP script**:

```php
<?php
$user = findUser($_GET["id"]);
echo json_encode($user);
```

In Manaknight, that becomes:

```lang
api GET /users/:id {
  uses { db }

  match db.findUser(id) {
    some(user) -> respond json(user)
    none -> respond status 404
  }
}
```

Same mental model:

* no persistence
* no shared memory
* response is explicit

But now:

* types are enforced
* errors are explicit
* side effects are declared

---

## 6️⃣ Concurrency Comparison (Important)

| Runtime    | Isolation   | Cost per request |
| ---------- | ----------- | ---------------- |
| PHP-FPM    | OS process  | High             |
| Node.js    | Shared heap | Low (but risky)  |
| Manaknight | VM          | Very low         |

Manaknight gives you:

* PHP-FPM semantics
* Node-level efficiency
* WASM-level isolation

That’s the sweet spot.

---

## 7️⃣ Scaling Story (Just Like PHP)

You scale Manaknight **exactly like PHP**:

```text
[ Load Balancer ]
    ↓
[ Manaknight #1 ]
[ Manaknight #2 ]
[ Manaknight #3 ]
```

* No sticky sessions
* No shared cache
* Stateless by default
* Horizontal scale is trivial

---

## 8️⃣ One Critical Difference vs PHP (Important)

PHP lets you do this:

```php
$GLOBALS["x"] = 5;
```

Manaknight **will not let you cheat**.

* No globals
* No static state
* No hidden caches
* No request bleed

That’s why Manaknight is suitable for:

* banking
* fintech
* audit-heavy systems

Where PHP often isn’t.

---

## 9️⃣ One-Sentence Answer (The Truth)

> **Yes — Manaknight is a PHP-FPM–style request/response system, but with VM-level isolation, compiler-enforced safety, and deterministic execution.**

Or even shorter:

> **Manaknight is “PHP done right.”**

# 1️⃣ Designing a FastCGI Adapter for Manaknight

## 1.1 What FastCGI Actually Is (Important)

FastCGI is just:

* a **binary protocol**
* over **Unix socket or TCP**
* that sends:

  * request metadata (method, path, headers)
  * body
* and receives:

  * status
  * headers
  * body

PHP-FPM is just **one FastCGI server**.

👉 We are building **Manaknight-FPM**.

---

## 1.2 Architecture Overview

```text
┌──────────┐     FastCGI      ┌────────────────────┐
│  NGINX   │ ──────────────▶ │ manaknight-fpm      │
│          │                 │ (FastCGI adapter)   │
└──────────┘                 │                    │
                             │  ┌──────────────┐  │
                             │  │ VM per req   │  │
                             │  │ MicroQuickJS│  │
                             │  └──────────────┘  │
                             └────────────────────┘
```

Key point:

> **FastCGI does NOT run Manaknight logic.**
> It only *adapts requests* into the Manaknight runtime.

---

## 1.3 Process Model (PHP-Compatible but Safer)

We copy the **PHP-FPM master/worker model**, but improve it.

### Master Process

* Opens FastCGI socket
* Preloads:

  * bytecode
  * route manifest
  * OpenAPI schemas
* Spawns workers

### Worker Process

* Handles FastCGI requests
* For **each request**:

  * creates a **new VM**
  * executes bytecode
  * destroys VM

No shared heap between requests.

---

## 1.4 Worker Loop (Core Logic)

Pseudo-code (language-agnostic):

```pseudo
while true:
  req = fastcgi_accept()

  route = match_route(req.method, req.path)
  if route == none:
    fastcgi_respond(404)
    continue

  params = validate_params(route, req)
  if params.error:
    fastcgi_respond(400)
    continue

  vm = create_vm(
    memory_limit = 256KB,
    cpu_budget = 5ms
  )

  effects = inject_effects(route.effects)

  result = vm.execute(route.bytecode, params, effects)

  destroy_vm(vm)

  if result.error:
    fastcgi_respond(500)
  else:
    fastcgi_respond(
      status = result.status,
      headers = result.headers,
      body = result.body
    )
```

---

## 1.5 Why This Is Safer Than PHP-FPM

| PHP-FPM                   | Manaknight-FPM                |
| ------------------------- | ----------------------------- |
| PHP interpreter reused    | VM recreated per request      |
| Globals exist             | Globals impossible            |
| Extensions run in-process | Effects are capability-scoped |
| Memory leaks accumulate   | Memory freed per request      |
| Fatal error kills worker  | VM failure only               |

---

## 1.6 Socket Layout

Exactly like PHP:

```text
/run/
 ├─ php-fpm.sock
 └─ manaknight-fpm.sock
```

---

# 2️⃣ Running Manaknight Behind NGINX

## 2.1 NGINX Configuration

### Basic API config

```nginx
server {
    listen 80;
    server_name api.example.com;

    location / {
        include fastcgi_params;

        fastcgi_pass unix:/run/manaknight-fpm.sock;
        fastcgi_param REQUEST_METHOD  $request_method;
        fastcgi_param REQUEST_URI     $request_uri;
        fastcgi_param CONTENT_TYPE    $content_type;
        fastcgi_param CONTENT_LENGTH  $content_length;

        fastcgi_param HTTP_HOST       $host;
        fastcgi_param HTTP_AUTHORIZATION $http_authorization;

        fastcgi_read_timeout 10s;
    }
}
```

That’s it.

NGINX:

* handles TLS
* handles buffering
* handles slow clients
* handles keepalive

Manaknight:

* executes logic
* enforces safety

---

## 2.2 Zero-Downtime Reloads

Same strategy as PHP:

```bash
# Start new adapter
manaknight-fpm --config prod.toml --graceful

# Reload NGINX
nginx -s reload

# Stop old adapter
kill -TERM <old-master-pid>
```

No in-flight requests lost.

---

## 2.3 Worker Scaling

```text
manaknight-fpm:
  workers: 8
  max_requests_per_worker: unlimited
```

Unlike PHP:

* you do **not** need `max_requests` to prevent leaks
* workers can run indefinitely

---

# 3️⃣ Memory Usage: Manaknight vs PHP (Real Comparison)

This is where things get very interesting.

---

## 3.1 Per-Request Memory Cost

### PHP-FPM (realistic production)

| Component               | Memory    |
| ----------------------- | --------- |
| PHP interpreter         | ~20–30 MB |
| Loaded extensions       | ~10–30 MB |
| Opcode cache            | ~20 MB    |
| Per-request allocations | ~1–5 MB   |

➡ **~40–80 MB per worker**

Yes — that’s why PHP servers are RAM-hungry.

---

### Manaknight (MicroQuickJS-based)

| Component          | Memory         |
| ------------------ | -------------- |
| Runtime binary     | ~2–5 MB        |
| Preloaded bytecode | ~1–3 MB        |
| Per-request VM     | **100–500 KB** |
| Effects objects    | ~50 KB         |

➡ **~150–600 KB per request**

That’s not a typo.

---

## 3.2 1,000 Concurrent Requests

| Runtime    | Approx RAM                  |
| ---------- | --------------------------- |
| PHP-FPM    | ❌ impossible (80+ GB)       |
| Node.js    | 1–5 GB (shared heap growth) |
| Manaknight | **150–600 MB (hard cap)**   |

This is why:

* PHP scales by adding servers
* Node scales until it suddenly dies
* Manaknight scales *predictably*

---

## 3.3 Why Manaknight Uses So Little Memory

* No JIT
* No reflection
* No dynamic loading
* No global symbol tables
* No opcache
* No request persistence
* No user-defined globals

Every request:

* gets a tiny heap
* is killed
* memory is returned immediately

---

## 3.4 Failure Memory Behavior

| Failure      | PHP            | Manaknight |
| ------------ | -------------- | ---------- |
| Memory leak  | Accumulates    | Impossible |
| Fatal error  | Kills worker   | VM dies    |
| Bad request  | Corrupts state | Isolated   |
| Long request | Blocks worker  | Killed     |

---

# 4️⃣ When FastCGI Makes Sense vs Native Runtime

### Use FastCGI if:

* You already have NGINX everywhere
* You want PHP-like ops familiarity
* You want easy side-by-side PHP + Manaknight
* You want zero change to infra

### Use Native Runtime if:

* You want max throughput
* You want fewer layers
* You want simpler deployment
* You’re building greenfield

Both are valid.
FastCGI is a **compatibility bridge**, not a requirement.
