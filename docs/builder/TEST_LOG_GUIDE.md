# Builder Test Log Guide

## 文档目的

本文档规定 Builder 每次正式测试后应如何记录测试结果。测试日志的目的不是堆输出，而是让 Leader 和 Reviewer 快速判断：本次实现是否真的构建过、测过、失败在哪里、是否有复现命令。

## 是否需要生成测试日志

需要。每次正式测试后都应生成记录，无论成功或失败。

原因：

- 成功日志证明功能被验证过，避免只口头说明。
- 失败日志保留现场，方便 Reviewer 和后续 Builder 复现。
- 多轮实现时可以比较问题是否回归。
- 面试整理时可以提炼工程化验证过程。

## 日志存放策略

推荐分两层记录：

| 类型 | 推荐位置 | 是否长期保留 | 内容 |
|---|---|---|---|
| 原始命令输出 | `build/test_logs/` | 不强制长期保留 | 构建、ctest、curl、smoke test 的完整输出 |
| 交付摘要 | `docs/builder/Mx_TEST_REPORT.md` | 建议保留 | 本轮测试命令、结果、失败原因、修复状态 |

说明：

- `build/test_logs/` 适合保存机器生成的原始日志，通常不作为核心设计文档维护。
- `docs/builder/Mx_TEST_REPORT.md` 是给 Leader 和 Reviewer 看的摘要，必须简洁、可复现。
- 如果项目后续加入 `.gitignore`，可忽略 `build/`，但保留 `docs/builder/Mx_TEST_REPORT.md`。

## M1 测试报告模板

Builder 完成 M1 后，应创建或更新：

```text
docs/builder/M1_TEST_REPORT.md
```

推荐内容：

```markdown
# M1 Test Report

## 测试环境

| 项目 | 内容 |
|---|---|
| OS | 例如 Ubuntu 22.04 |
| Compiler | 例如 g++ 11.4 |
| CMake | 例如 3.22 |
| 测试时间 | YYYY-MM-DD HH:MM |

## 测试结果汇总

| 测试项 | 命令 | 结果 | 备注 |
|---|---|---|---|
| 配置构建 | `cmake -S . -B build` | PASS/FAIL |  |
| 编译 | `cmake --build build` | PASS/FAIL |  |
| 单元测试 | `ctest --test-dir build` | PASS/FAIL |  |
| 集成测试 | `tests/smoke_test.sh` | PASS/FAIL |  |

## 失败记录

| 问题 | 复现命令 | 原因 | 处理状态 |
|---|---|---|---|
| 无 |  |  |  |

## 关键输出

只粘贴关键行，例如：

```text
100% tests passed
HTTP/1.1 200 OK
HTTP/1.1 404 Not Found
```
```

## 记录原则

- PASS 和 FAIL 都要记录。
- 不要把几千行完整日志粘进 Markdown，只保留关键输出和原始日志路径。
- 失败必须写复现命令。
- 如果跳过某项测试，必须说明是工具缺失、环境限制，还是当前里程碑不要求。
- 不允许用“应该没问题”代替测试结果。
