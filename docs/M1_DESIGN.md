# M1 Design: Minimal HTTP Server

## 文档目的

本文档用于指导 Builder 实现 M1 阶段。Builder 在开始编码前，应先阅读项目根目录的 `AGENTS.md`，再阅读本文档。

M1 的目标是建立一个可运行、可验证、可继续演进的最小 HTTP Server。它不是最终高性能版本，但必须从第一阶段就采用正确的 Linux 网络编程基础：C++20、CMake、非阻塞 socket、epoll 事件循环和清晰的模块边界。

## M1 定位

M1 要实现的是单线程、非阻塞、epoll 驱动的 HTTP/1.1 静态文件服务器。

请求链路如下：

```text
浏览器 / curl
    |
    v
TCP 连接
    |
    v
单线程 epoll 事件循环
    |
    v
读取 HTTP 请求
    |
    v
解析 GET 请求
    |
    v
返回静态文件或错误响应
```

M1 的核心价值：

- 跑通从 TCP 连接建立到 HTTP 响应返回的完整链路。
- 为 M2 的 Reactor 抽象提供可重构基础。
- 为 M3 的 HTTP 状态机、M4 的线程模型、M5 的性能优化保留接口空间。
- 避免写成一次性 demo。

## M1 精简设计表

| 项目 | 内容 |
|---|---|
| 需求目标 | 搭建项目骨架，实现单线程、非阻塞、epoll 驱动的最小 HTTP/1.1 静态文件服务器 |
| 使用场景 | 用户通过浏览器或 curl 访问 `127.0.0.1:8080/index.html`，服务器返回静态 HTML/CSS/文本文件 |
| 技术方案 | C++20 + CMake + Linux socket + epoll LT 模式 + 非阻塞 IO + 基础 HTTP GET 解析 |
| 选型理由 | M1 要先跑通完整请求链路，epoll 能从一开始建立正确网络模型；LT 模式比 ET 更容易保证正确性，适合 MVP |
| 模块影响 | `app`、`net`、`http`、`base`、`tests`、`public`、`docs` |
| 实现边界 | 支持 GET、静态文件、基础错误码；暂不做线程池、主从 Reactor、keep-alive、sendfile、异步日志 |
| 验收标准 | CMake 可构建；curl 可访问首页；不存在请求时不忙等；非法请求返回 400；不存在文件返回 404 |
| 风险与对策 | 半包处理不足、fd 泄漏、路径穿越、阻塞读写；M1 用简单 Buffer、RAII fd 包装、路径限制和非阻塞读写规避 |

## 功能拆解

| 功能 | 必须程度 | 说明 |
|---|---|---|
| 项目目录结构 | 必须 | 后续所有模块都依赖清晰目录，不然后面重构成本会高 |
| CMake 构建 | 必须 | 秋招项目需要能被别人一键构建，不能只靠 IDE |
| TCP 监听 | 必须 | 完成 socket、bind、listen、accept 基础链路 |
| fd 非阻塞 | 必须 | 从 M1 开始建立高性能服务器的基本习惯 |
| epoll 事件循环 | 必须 | M1 就使用 epoll，避免后续从阻塞模型大改 |
| HTTP GET 解析 | 必须 | 只解析请求行和必要头部，不追求完整协议 |
| 静态文件返回 | 必须 | 能体现 server 的实际可用性 |
| 错误响应 | 必须 | 至少支持 400、404、500，建议同时支持 403、405 |
| 基础测试 | 必须 | 覆盖 HTTP 解析、响应构造、路径安全和端到端访问 |
| 简单日志 | 建议 | M1 用同步 `stdout/stderr` 即可，异步日志留到 M5 |
| README | 必须 | 写明构建、运行、curl 验证命令 |

## 推荐目录结构

Builder 应按以下结构组织 M1，后续里程碑可在此基础上演进：

```text
HP_HTTP_Server/
├── CMakeLists.txt
├── README.md
├── AGENTS.md
├── app/
│   └── main.cpp
├── include/
│   ├── base/
│   │   ├── NonCopyable.h
│   │   └── Logger.h
│   ├── net/
│   │   ├── Socket.h
│   │   ├── InetAddress.h
│   │   ├── Epoller.h
│   │   └── TcpServer.h
│   └── http/
│       ├── HttpRequest.h
│       ├── HttpResponse.h
│       └── HttpServer.h
├── src/
│   ├── base/
│   │   └── Logger.cpp
│   ├── net/
│   │   ├── Socket.cpp
│   │   ├── InetAddress.cpp
│   │   ├── Epoller.cpp
│   │   └── TcpServer.cpp
│   └── http/
│       ├── HttpRequest.cpp
│       ├── HttpResponse.cpp
│       └── HttpServer.cpp
├── public/
│   ├── index.html
│   └── 404.html
├── tests/
└── docs/
    └── M1_DESIGN.md
```

M1 不建议直接引入 `EventLoop`、`Channel`、`Acceptor`。这些属于 M2 的 Reactor 抽象。如果 M1 过早抽象，容易出现类很多但主链路不稳的问题。M1 应先把 socket、epoll、HTTP 静态文件链路跑通；M2 再把成熟链路抽象成 Reactor。

## 模块职责

| 模块 | M1 职责 | 不负责 |
|---|---|---|
| `app` | 读取启动参数，创建服务器，启动事件循环 | 不处理网络细节 |
| `net::Socket` | 封装 fd 创建、bind、listen、accept、close、设置非阻塞 | 不解析 HTTP |
| `net::InetAddress` | 封装 IP、端口、`sockaddr_in` | 不管理连接生命周期 |
| `net::Epoller` | 封装 `epoll_create1`、`epoll_ctl`、`epoll_wait` | 不决定业务逻辑 |
| `net::TcpServer` | 管理监听 fd、客户端 fd、epoll 主循环、读写事件 | 不关心具体 HTTP 语义细节 |
| `http::HttpRequest` | 保存 method、path、version、headers | 不负责 socket 读写 |
| `http::HttpResponse` | 构造状态行、响应头、响应体 | 不负责 epoll 事件管理 |
| `http::HttpServer` | 把原始请求字符串转成 HTTP 响应 | 不直接操作 epoll |
| `base::Logger` | M1 简单日志输出 | 不做异步、不做落盘队列 |

## 核心调用链

```text
main()
  -> HttpServer server(port, static_root)
  -> server.start()
      -> create listen socket
      -> set non-blocking
      -> bind/listen
      -> add listen fd to epoll
      -> epoll_wait loop
          -> listen fd readable: accept new client
          -> client fd readable: read request
          -> HttpRequest parse
          -> HttpResponse build
          -> write response
          -> close client fd
```

M1 默认每个请求处理完就关闭连接：

```http
Connection: close
```

原因：M1 的目标是跑通链路。keep-alive 会引入连接复用、请求边界、多次读写状态管理，应放到 M3 或 M4。

## HTTP 支持范围

| 能力 | M1 是否支持 | 说明 |
|---|---|---|
| GET | 支持 | M1 唯一必须支持的方法 |
| POST | 不支持 | 返回 405 Method Not Allowed |
| HTTP/1.0 | 可接受 | 可以简单返回响应，但不重点适配 |
| HTTP/1.1 | 支持基础格式 | 不做完整 RFC 覆盖 |
| keep-alive | 不支持 | 统一 `Connection: close` |
| 静态文件 | 支持 | 只允许访问 `public/` 下文件 |
| 目录访问 | 简化支持 | `/` 映射到 `/index.html` |
| MIME 类型 | 基础支持 | html、css、js、png、jpg、jpeg、txt、json |
| Range 请求 | 不支持 | 后续优化项 |
| chunked | 不支持 | 后续协议增强项 |

## 错误码设计

| 状态码 | 触发场景 |
|---|---|
| 200 OK | 文件存在且可读取 |
| 400 Bad Request | 请求行格式错误、HTTP 格式明显非法 |
| 403 Forbidden | 请求路径试图访问 `../` 或静态目录外文件 |
| 404 Not Found | 文件不存在 |
| 405 Method Not Allowed | 非 GET 方法 |
| 500 Internal Server Error | 文件读取失败、系统调用异常等 |

## 路径安全设计

静态文件服务器必须防路径穿越。例如用户请求：

```text
GET /../../etc/passwd HTTP/1.1
```

M1 必须拒绝。

推荐策略：

| 步骤 | 说明 |
|---|---|
| URL decode | M1 可以先只做基础 `%20` 等处理，复杂情况后续补 |
| 拼接路径 | `static_root + request_path` |
| 规范化路径 | 使用 `std::filesystem::weakly_canonical` 或等价方式 |
| 边界判断 | 规范化后的路径必须仍然位于 `public/` 内 |
| 失败处理 | 越界返回 403 |

这个点适合面试讲：即使是 MVP，也不能把安全问题完全忽略。

## 非阻塞 IO 策略

M1 使用 epoll LT 模式。

| 方案 | 是否选择 | 原因 |
|---|---|---|
| 阻塞 IO + accept/read/write | 不选 | 虽然简单，但和高性能项目定位不匹配 |
| select/poll | 不选 | 面试价值低于 epoll，且不是 Linux 高并发主流方案 |
| epoll LT | 选择 | 正确性优先，适合 M1，后续可平滑演进 |
| epoll ET | 暂不选 | 性能更好但边界复杂，适合 M2/M5 再讨论 |

M1 的读写处理原则：

| 场景 | 处理方式 |
|---|---|
| `read > 0` | 追加到连接 buffer，尝试解析请求 |
| `read == 0` | 对端关闭连接，移除 epoll 并 close fd |
| `read < 0 && errno == EAGAIN` | 当前无更多数据，等待下次事件 |
| `read < 0 && errno == EINTR` | 重试 |
| 其他 read 错误 | 关闭连接 |
| `write < response_size` | 把剩余数据保留到连接状态中，并关注 `EPOLLOUT` |
| write 出错 | 关闭连接 |

严格来说，高性能服务器不能假设一次 `write` 写完响应。因此 M1 建议实现一个最小输出缓冲：如果没写完，把剩余数据放到连接状态里，并注册写事件。

## 连接状态设计

M1 可以用一个简单结构维护每个客户端 fd：

```cpp
struct Connection {
    int fd;
    std::string read_buffer;
    std::string write_buffer;
    bool response_ready;
};
```

字段职责：

| 字段 | 作用 |
|---|---|
| `fd` | 客户端 socket |
| `read_buffer` | 保存可能分片到达的 HTTP 请求 |
| `write_buffer` | 保存尚未写完的响应 |
| `response_ready` | 标记是否已经生成响应 |

判断 HTTP 请求是否完整，M1 可以先以：

```text
\r\n\r\n
```

作为请求头结束标志。因为 M1 只支持 GET，没有请求体，所以读到请求头结束即可处理。

## M1 暂不实现

| 暂不实现 | 原因 |
|---|---|
| 主从 Reactor | M2 核心内容，M1 先验证单 Reactor 链路 |
| 线程池 | M1 静态文件规模小，引入线程池会干扰主线 |
| sendfile | M5 性能优化点，需要先有基准数据 |
| 异步日志 | M5 做，否则 M1 复杂度上升 |
| 定时器 | M4 做空闲连接治理 |
| keep-alive | M3/M4 做，请求边界和连接生命周期更复杂 |
| 完整 HTTP 解析器 | M3 做状态机，M1 只做最小 GET 解析 |
| 压测报告 | M1 只做 smoke test，正式 wrk 压测从 M2/M5 开始 |

## 测试设计

M1 需要测试，但测试范围应服务于 MVP 正确性，不追求复杂测试框架和大规模压测。测试重点是：解析正确、响应正确、路径安全、服务可访问。

推荐测试结构：

```text
tests/
├── CMakeLists.txt
├── http_request_test.cpp
├── http_response_test.cpp
├── static_file_test.cpp
└── smoke_test.sh
```

M1 可以先使用 C++ 标准库 `assert` 编写轻量单元测试，不强制引入 GoogleTest。原因是当前阶段项目还很小，引入第三方测试框架会增加构建和依赖复杂度。后续 M3 开始 HTTP 状态机复杂度上升时，再评估是否引入 GoogleTest。

测试分层如下：

| 类型 | 是否需要 | 覆盖内容 | 验证方式 |
|---|---|---|---|
| 单元测试 | 需要 | HTTP 请求解析、响应构造、路径安全函数 | CMake 构建测试可执行文件并运行 |
| 集成测试 | 需要 | 启动 server 后通过 curl 访问真实端口 | `tests/smoke_test.sh` |
| 压力测试 | 暂不需要 | 高并发 QPS、P99 延迟 | M2/M5 再使用 wrk |
| 内存检测 | 建议 | fd 和堆内存是否明显泄漏 | 有 valgrind 时可选执行 |

必须覆盖的单元测试用例：

| 测试对象 | 用例 | 期望 |
|---|---|---|
| `HttpRequest` | `GET / HTTP/1.1` | method 为 GET，path 为 `/` |
| `HttpRequest` | `GET /index.html HTTP/1.1` | path 为 `/index.html` |
| `HttpRequest` | 请求行字段不足 | 解析失败或返回 400 |
| `HttpRequest` | `POST / HTTP/1.1` | 解析成功但后续响应为 405 |
| `HttpResponse` | 200 响应 | 包含状态行、Content-Length、Connection: close |
| `HttpResponse` | 404 响应 | 状态码和响应体正确 |
| 静态文件路径 | `/` | 映射到 `public/index.html` |
| 静态文件路径 | `/../../etc/passwd` | 返回 403 或解析失败 |
| 静态文件路径 | 不存在文件 | 返回 404 |

必须覆盖的集成测试用例：

| 场景 | 命令 | 期望 |
|---|---|---|
| 首页访问 | `curl -i http://127.0.0.1:8080/` | 返回 200 |
| 指定文件访问 | `curl -i http://127.0.0.1:8080/index.html` | 返回 200 |
| 不存在文件 | `curl -i http://127.0.0.1:8080/not-exist` | 返回 404 |
| 非 GET 方法 | `curl -i -X POST http://127.0.0.1:8080/` | 返回 405 |
| 路径穿越 | `curl -i http://127.0.0.1:8080/../../etc/passwd` | 返回 403 或 400 |

测试交付要求：

- `cmake --build build` 应同时构建 server 和测试目标。
- README 中必须写明如何运行单元测试和 smoke test。
- Builder 完成 M1 后，必须汇报构建结果、单元测试结果和 curl 集成测试结果。
- 如果某个测试因本机缺少工具无法运行，Builder 必须说明原因，不允许静默跳过。

## 验收标准

M1 必须满足以下命令级验收：

| 验收项 | 验证方式 | 期望结果 |
|---|---|---|
| 构建成功 | `cmake -S . -B build && cmake --build build` | 生成 server 可执行文件 |
| 启动成功 | `./build/hp_http_server --port 8080 --root public` | 监听 8080 |
| 首页访问 | `curl -i http://127.0.0.1:8080/` | 返回 `200 OK` 和 `index.html` |
| 文件访问 | `curl -i http://127.0.0.1:8080/index.html` | 返回 `200 OK` |
| 404 | `curl -i http://127.0.0.1:8080/not-exist` | 返回 `404 Not Found` |
| 非 GET | `curl -i -X POST http://127.0.0.1:8080/` | 返回 `405 Method Not Allowed` |
| 非法路径 | `curl -i http://127.0.0.1:8080/../../etc/passwd` | 返回 `403 Forbidden` 或 `400 Bad Request` |
| 多连接 | 连续执行多次 curl | 服务不崩溃 |
| 空闲状态 | 启动后无请求 | CPU 不应持续占满 |
| 单元测试 | `ctest --test-dir build` 或等价命令 | HTTP 解析、响应构造、路径安全测试通过 |
| 集成测试 | `tests/smoke_test.sh` | curl 场景全部通过 |

## Builder 实现任务说明

Builder 的任务边界如下：

> 阅读 `AGENTS.md` 和 `docs/M1_DESIGN.md`。实现 M1：基于 C++20 + CMake 创建单线程 epoll HTTP Server。只支持 HTTP GET 静态文件访问，默认 `Connection: close`，支持 200/400/403/404/405/500，必须使用非阻塞 socket 和 epoll LT。不要实现线程池、主从 Reactor、sendfile、异步日志、keep-alive。完成后提供构建命令、单元测试结果和 curl 集成测试结果。

Builder 必须遵守：

- 不要越界实现 M2/M3/M4/M5 功能。
- 不要引入第三方网络库。
- 不要使用阻塞式一连接一线程模型。
- 不要把 HTTP 解析写死成只适配某一个 curl 请求。
- 不要允许访问 `public/` 目录之外的文件。
- 新增代码应保持模块边界清晰，便于 M2 重构 Reactor。

## M1 产出物

| 产出物 | 说明 |
|---|---|
| 源码骨架 | `app/`、`include/`、`src/` |
| 构建文件 | `CMakeLists.txt` |
| 静态资源 | `public/index.html`、`public/404.html` |
| 测试代码 | HTTP 解析、响应构造、路径安全、smoke test |
| 使用文档 | `README.md` |
| 设计文档 | `docs/M1_DESIGN.md` |
| 验收记录 | README 或 docs 中记录 curl 验证命令 |
