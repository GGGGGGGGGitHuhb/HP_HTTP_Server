# M1 Review Report

## Findings

| 严重级别 | 文件/位置 | 问题 | 影响 | 建议 |
|---|---|---|---|---|
| Medium | `src/net/TcpServer.cpp:43` | 当 epoll 事件同时包含 `EPOLLIN` 和 `EPOLLRDHUP` 时，当前主循环先命中 `EPOLLRDHUP` 分支并直接 `closeConnection(fd)`，没有读取 socket 中已经到达的请求数据。触发条件：客户端发送完整 HTTP 请求后立即半关闭写端，例如 `shutdown(SHUT_WR)`。 | 服务端会丢弃已经到达的完整请求，客户端收到空响应。该行为会影响真实 TCP 客户端、测试工具或代理在发送请求后主动半关闭写端的场景，不符合 M1 对 read 事件完整处理和稳定短连接的要求。 | 调整 `net::TcpServer` 的事件处理顺序：当事件包含 `EPOLLIN` 时先执行读处理并尝试生成响应；`EPOLLRDHUP` 只作为对端关闭提示，不应在可读数据未 drain 前直接关闭。建议补充半关闭集成用例：启动 server，客户端发送 `GET / HTTP/1.1\r\nHost: localhost\r\n\r\n` 后调用 `shutdown(SHUT_WR)`，验收标准为仍返回 `HTTP/1.1 200 OK` 和非空响应体。 |

## Verification

| 验证项 | 结果 | 备注 |
|---|---|---|
| 配置构建 | PASS | 已执行 `cmake -S . -B build`，配置生成成功。 |
| 编译 | PASS | 已执行 `cmake --build build`，输出 `ninja: no work to do.`，说明当前 build 产物已是最新。 |
| 单元测试 | PASS | 已执行 `ctest --test-dir build --output-on-failure`，3/3 测试通过。 |
| 集成测试 | PASS | 已执行 `./tests/smoke_test.sh`，输出 `smoke test passed`。 |
| 半关闭复现 | FAIL | 已启动 `./build/hp_http_server --port 18081 --root public`，再执行 `python3 -c 'import socket; s=socket.create_connection(("127.0.0.1",18081),2); s.sendall(b"GET / HTTP/1.1\r\nHost: localhost\r\n\r\n"); s.shutdown(socket.SHUT_WR); data=s.recv(4096); print(repr(data[:80])); print(len(data)); s.close()'`，结果为 `b''` 和 `0`。 |

## Summary

| 项目 | 结论 |
|---|---|
| 是否建议通过 | 暂不建议通过。 |
| 是否需要 Builder 返工 | 需要。建议优先修复 `EPOLLIN + EPOLLRDHUP` 同时到达时丢请求的问题。 |
| 当前达标项 | 项目结构、CMake 构建、非阻塞 listen/client fd、epoll LT、accept 循环、短写缓冲、fd 删除与关闭、GET/静态文件/错误码/路径安全、`Connection: close`、Builder 测试记录均基本符合 M1 要求。 |
| 剩余风险 | 现有 smoke 测试覆盖常规 curl 请求，但未覆盖客户端半关闭、慢速分片请求、异常断连等网络边界。本轮只将已复现的半关闭丢响应作为 M1 当前问题提出，不要求提前实现 M2/M3/M4/M5 能力。 |
