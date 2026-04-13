# M1 Review Guide

## 文档目的

本文档用于指导 Reviewer 高效审查 M1 Builder 的实现结果。Reviewer 应重点判断实现是否满足 `docs/builder/M1_DESIGN.md`，而不是要求 Builder 提前实现 M2/M3/M4/M5 的能力。

## Reviewer 输入材料

Reviewer 开始前应阅读：

| 顺序 | 文件 | 目的 |
|---|---|---|
| 1 | `AGENTS.md` | 理解项目长期定位、技术栈和决策原则 |
| 2 | `docs/builder/M1_DESIGN.md` | 理解 M1 的功能范围、模块边界和验收标准 |
| 3 | `docs/builder/TEST_LOG_GUIDE.md` | 理解 Builder 应如何提交测试记录 |
| 4 | `docs/builder/M1_TEST_REPORT.md` | 查看 Builder 的实际测试结果，若文件不存在则记为问题 |
| 5 | 本次代码变更 | 审查实现是否符合设计 |

## 审查结论格式

Reviewer 的输出应优先列问题，按严重程度排序。每次审查完成后，Reviewer 必须在本目录 `docs/reviewer/` 下新增一份 Markdown 审查记录文件，不只在对话中口头汇报。

推荐文件命名：

```text
M1_REVIEW_REPORT_YYYYMMDD_HHMM.md
```

如果同一天有多轮审查，可以追加简短后缀：

```text
M1_REVIEW_REPORT_YYYYMMDD_HHMM_half_close.md
```

审查记录必须至少包含以下三个章节：

推荐格式：

```markdown
# M1 Review Report

## Findings

| 严重级别 | 文件/位置 | 问题 | 影响 | 建议 |
|---|---|---|---|---|
| High/Medium/Low | path:line |  |  |  |

## Verification

| 验证项 | 结果 | 备注 |
|---|---|---|
| 构建 | PASS/FAIL/未运行 |  |
| 单元测试 | PASS/FAIL/未运行 |  |
| 集成测试 | PASS/FAIL/未运行 |  |

## Summary

简短总结是否建议通过，以及剩余风险。
```

记录要求：

- `Findings` 必须写清楚问题位置、严重级别、触发条件、影响和修改建议。
- `Verification` 必须写清楚实际执行过的命令、PASS/FAIL/未运行，以及失败复现方式。
- `Summary` 必须明确是否建议通过、是否需要 Builder 返工，以及剩余风险。
- 如果发现问题，Reviewer 必须给出面向 Builder 的修改意见，并写入审查记录。修改意见应说明期望调整的模块、核心处理原则、建议补充的验证用例和验收标准。
- 如果审查发现问题，应保留最小复现命令或步骤，方便 Builder 直接验证修复。
- 如果某项验证未运行，必须说明原因，不能留空。
- 审查记录文件与本指南同级存放，便于 Leader 汇总每轮 Reviewer 反馈。

如果没有发现问题，应明确写：

```text
No blocking issues found.
```

并说明仍未覆盖的测试或残余风险。

## M1 必审范围

| 审查项 | 通过标准 |
|---|---|
| 项目结构 | 符合 `app`、`include`、`src`、`tests`、`public`、`docs` 的基本划分 |
| 构建系统 | CMake 能配置、编译 server 和测试目标 |
| 非阻塞 socket | listen fd 和 client fd 应设置为 non-blocking |
| epoll 使用 | 使用 epoll LT 模式，正确 add/mod/del fd |
| accept 处理 | 能处理多个连接，不因单个连接阻塞主循环 |
| read 处理 | 正确处理 `read > 0`、`read == 0`、`EAGAIN`、`EINTR` 和错误 |
| write 处理 | 不默认一次写完；短写时应保留剩余数据或给出明确合理边界 |
| fd 生命周期 | 关闭连接时从 epoll 删除并 close fd，避免 fd 泄漏 |
| HTTP GET | 正确解析基础请求行和路径 |
| 错误响应 | 支持 400、403、404、405、500 中 M1 要求的场景 |
| 路径安全 | 不能访问 `public/` 目录之外的文件 |
| 短连接策略 | 默认 `Connection: close`，不偷偷实现未设计的 keep-alive |
| 测试记录 | Builder 提供构建、单元测试、集成测试结果 |

## 高风险问题清单

Reviewer 应优先检查以下问题：

| 风险 | 常见表现 | 严重级别 |
|---|---|---|
| 路径穿越 | `GET /../../etc/passwd` 可读取系统文件 | High |
| 阻塞 IO | client fd 未设置 non-blocking，慢连接卡住服务 | High |
| fd 泄漏 | 错误路径未 close fd，重复请求后 fd 增长 | High |
| 忙等 | 无请求时 CPU 持续升高 | High |
| 忽略短写 | 大响应只写出一部分，客户端收到截断内容 | Medium |
| HTTP 解析过度硬编码 | 只能处理某一种 curl 请求格式 | Medium |
| 缺少测试记录 | Builder 未证明构建和测试已执行 | Medium |
| 越界实现 | 在 M1 中提前引入线程池、主从 Reactor、keep-alive 等复杂能力 | Low/Medium |

## 推荐验证命令

Reviewer 可以复用 Builder 的命令复核：

```bash
cmake -S . -B build
cmake --build build
ctest --test-dir build
./build/hp_http_server --port 8080 --root public
```

在另一个终端执行：

```bash
curl -i http://127.0.0.1:8080/
curl -i http://127.0.0.1:8080/index.html
curl -i http://127.0.0.1:8080/not-exist
curl -i -X POST http://127.0.0.1:8080/
curl -i http://127.0.0.1:8080/../../etc/passwd
```

期望结果：

| 命令场景 | 期望 |
|---|---|
| 首页 | 200 |
| 指定文件 | 200 |
| 不存在文件 | 404 |
| 非 GET | 405 |
| 路径穿越 | 403 或 400 |

## 不应要求 Builder 在 M1 完成的内容

Reviewer 不应把以下内容作为 M1 阻塞问题：

- 主从 Reactor。
- 线程池。
- sendfile。
- 异步日志。
- keep-alive。
- 完整 HTTP 状态机。
- wrk 压测报告。
- perf 火焰图。

如果代码实现已经影响后续演进，可以作为建议提出；但只要不破坏 M1 目标，不应要求本阶段必须完成。
