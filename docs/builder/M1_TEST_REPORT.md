# M1 Test Report

## 测试环境

| 项目 | 内容 |
|---|---|
| OS | Linux Pengnuin 6.6.87.2-microsoft-standard-WSL2 x86_64 |
| Compiler | c++ (Ubuntu 13.3.0-6ubuntu2~24.04) 13.3.0 |
| CMake | cmake version 3.28.3 |
| 测试时间 | 2026-04-11 14:06 CST |

## 测试结果汇总

| 测试项 | 命令 | 结果 | 备注 |
|---|---|---|---|
| 环境记录 | `date`, `uname -a`, `c++ --version`, `cmake --version` | PASS | 原始日志：`build/test_logs/m1_environment.log` |
| 配置构建 | `cmake -S . -B build` | PASS | 原始日志：`build/test_logs/m1_configure.log` |
| 编译 | `cmake --build build` | PASS | 原始日志：`build/test_logs/m1_build.log` |
| 单元测试 | `ctest --test-dir build --output-on-failure` | PASS | 原始日志：`build/test_logs/m1_ctest.log` |
| 集成测试 | `./tests/smoke_test.sh` | PASS | 原始日志：`build/test_logs/m1_smoke.log`；需要允许本地 socket 监听和 curl 连接 |

## 失败记录

| 问题 | 复现命令 | 原因 | 处理状态 |
|---|---|---|---|
| 无 |  |  |  |

## 关键输出

```text
-- Build files have been written to: /home/power/projects/HP_HTTP_Server/build
[100%] Built target static_file_test
100% tests passed, 0 tests failed out of 3
smoke test passed
```

## 覆盖说明

- `http_request_test` 覆盖 GET、POST、非法请求行、非法 header。
- `http_response_test` 覆盖状态行、Content-Length、Connection: close 和基础 MIME。
- `static_file_test` 覆盖 `/` 到 `index.html`、正常文件、404、路径穿越和 `%2e%2e` 编码路径。
- `smoke_test.sh` 覆盖真实 server 启动后的首页、指定文件、404、405、403 和连续多连接 curl 请求。
