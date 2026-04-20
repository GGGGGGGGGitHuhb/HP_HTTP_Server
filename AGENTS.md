# HP_HTTP_Server Agent Guide

## Project

Build a resume-ready Linux C++ high-performance HTTP server.

Core stack:

- C++20
- CMake
- Linux socket API
- epoll-based non-blocking IO
- HTTP/1.1 static file service

All agents must keep the project focused on networking, concurrency, protocol handling, resource management, testing, performance analysis, and production-readiness.

## Required Reading

All agents must read this file before working.

Then read the role-specific guide:

| Role | Guide |
|---|---|
| Leader | `docs/leader/LEADER_GUIDE.md` |
| Builder | `docs/builder/BUILDER_GUIDE.md` |
| Reviewer | `docs/reviewer/REVIEWER_GUIDE.md` |

Role-specific milestone documents live under:

| Purpose | Path |
|---|---|
| Builder designs | `docs/builder/designs/` |
| Builder reports | `docs/builder/reports/` |
| Reviewer guides | `docs/reviewer/guides/` |
| Reviewer reports | `docs/reviewer/reports/` |
| Leader planning and reports | `docs/leader/` |
| User-facing overview | `docs/user/` |

## Role Boundaries

| Role | Responsibility |
|---|---|
| Leader | Plans milestones, owns architecture decisions, splits scope, maintains backlog, writes Builder and Reviewer instructions |
| Builder | Implements only the assigned milestone design, writes unit tests, runs required local verification, writes work/test reports |
| Reviewer | Reviews implementation against design, reruns or extends verification, records findings and review report |

Agents must not implement work outside the current milestone design unless the user explicitly changes the scope.

## Current Milestone Rules

- M1 is a single-threaded non-blocking epoll HTTP/1.1 static file server.
- M1 must not add thread pools, main-sub Reactor, keep-alive, sendfile, async logging, full HTTP parser, wrk reports, or deployment work unless a later design requires it.
- Later milestones must recover deferred items from `docs/leader/PROJECT_BACKLOG.md`.

## Documentation Rules

- `AGENTS.md` contains only broad rules that all agents need.
- Explanations for the user belong in `docs/user/`.
- Leader planning belongs in `docs/leader/`.
- Builder implementation details belong in `docs/builder/`.
- Reviewer checklists belong in `docs/reviewer/`.
- Each agent must write a work report after each assigned task. The chat response may be brief and point to the report.

## Common Commands

```bash
cmake -S . -B build
cmake --build build
ctest --test-dir build
tests/smoke_test.sh
```

## Formatting

- C++ standard is C++20.
- Use clang-format for C/C++ files.
- Repository format configuration is in `.clang-format`.
- VS Code format integration is in `.vscode/settings.json`.
- Do not reformat unrelated code in broad sweeps.

## Coding Style

Follow Google C++ style unless a local design document explicitly narrows a rule.

Naming:

- Types/classes/structs/enums: `PascalCase`, for example `TcpServer`, `HttpRequest`.
- Functions: `PascalCase`, for example `Start()`, `HandleReadEvent()`.
- Variables and parameters: `snake_case`, for example `client_fd`, `static_root`.
- Data members: `snake_case_`, for example `listen_socket_`, `read_buffer_`.
- Constants and enum values: `kPascalCase`, for example `kReadBufferSize`, `kInfo`.
- Namespaces: lower case, for example `hp::net`, `hp::http`.
- File names: lower case with underscores when needed, for example `tcp_server.h`.

Include order:

- The current file's matching header first.
- C system headers.
- C++ standard library headers.
- Third-party library headers.
- Project headers.
- Keep include groups separated by one blank line.

Class design:

- Prefer RAII for file descriptors, sockets, memory, and other owned resources.
- Put `public` before `private` unless there is a clear reason not to.
- Mark single-argument constructors `explicit` unless implicit conversion is intentional.
- Avoid broad global state; keep ownership and lifetime visible.
- Avoid complex macros; prefer typed constants, functions, and classes.
- Use `auto` only when it improves readability or the type is obvious from the right-hand side.

Header files:

- New header files should use `#pragma once`.
- Headers must be self-contained: including a header alone should be enough to compile it.
- Prefer forward declarations when they reduce unnecessary dependencies without hurting clarity.
- Keep public API declarations in headers and implementation details in `.cpp` files.
- Do not delete existing user comments unless they are clearly wrong and the current task requires fixing them.

## Safety

- Do not delete user changes.
- Do not bypass tests because a change looks small.
- Do not treat missing reports as acceptable handoff.
- If a command cannot run, record the reason in the relevant report.
