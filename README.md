# HP_HTTP_Server

A C++20 Linux HTTP/1.1 static file server for learning high-performance network programming. M1 implements a single-threaded, non-blocking, epoll LT based server with basic GET static file handling.

## Build

```bash
cmake -S . -B build
cmake --build build
```

## Run

```bash
./build/hp_http_server --port 8080 --root public
```

Options:

- `--port <port>`: listen port, default `8080`.
- `--root <dir>`: static file root, default `public`.

## Verify With curl

```bash
curl -i http://127.0.0.1:8080/
curl -i http://127.0.0.1:8080/index.html
curl -i http://127.0.0.1:8080/not-exist
curl -i -X POST http://127.0.0.1:8080/
curl -i --path-as-is http://127.0.0.1:8080/../../etc/passwd
```

Expected statuses: `200`, `200`, `404`, `405`, and `403`.

## Tests

```bash
ctest --test-dir build --output-on-failure
./tests/smoke_test.sh
```

`smoke_test.sh` builds the project if needed, starts the server on port `18080`, runs curl checks, and stops the server.

## M1 Scope

Implemented:

- TCP listen socket, bind, listen, and accept.
- Non-blocking file descriptors.
- Single-threaded epoll LT event loop.
- Minimal HTTP request parsing for request line and headers.
- GET static files from `public/` only.
- `200`, `400`, `403`, `404`, `405`, and `500` responses.
- Output buffering for partial non-blocking writes.
- Lightweight unit tests and curl smoke test.

Not implemented in M1: keep-alive, thread pool, master-sub Reactor, sendfile, async logger, timer, full HTTP state machine.
