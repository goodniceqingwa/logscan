# logscan

这是一个并行日志分析器的 C++17 框架骨架。当前只定义扩展点和数据契约，不提供读取、解析、分析、线程调度或输出功能。

代码中的注释以“调用方、所有权、线程边界和实现责任”为重点；它们描述的是框架契约，不代表默认实现已经存在。

## 处理边界

```text
LogSource -> RawLogBatch -> worker(parser + analyzer) -> collector -> LogReportSink
```

- `LogSource` 由协调线程调用，负责输入生命周期和原始批次。
- `LogParser`、`LogAnalyzer` 通过 `clone()` 为每个 worker 创建实例，避免默认共享可变状态。
- `LogReportSink` 在 collector 侧接收批次结果；是否按 `BatchId` 保序由 `LogScannerConfig` 约束。
- `LogScanner` 和 `src/ParallelExecutor.cpp` 仅留下编排、队列、取消、错误传播和汇聚的实现位置。

## 建议实现顺序

1. 实现一个具体的 `LogSource`。
2. 实现 `LogParser::clone()` 和 `parse()`。
3. 实现 `LogAnalyzer::clone()` 和 `analyze()`。
4. 实现 `LogReportSink`，明确批次结果的生命周期和顺序要求。
5. 在 `LogScanner.cpp` 中补齐线程池、有限容量队列、关闭/取消和错误传播。

## 产品目标

logscan 是一个面向开发者和运维人员的本地日志扫描工具。
它在不依赖外部服务的情况下，对结构化日志进行过滤、解析和统计。

## v0.1.0 使用方式

logscan <file> [--level <level>] [--contains <text>]

示例：

logscan app.log --level ERROR --contains timeout

## 支持的日志格式

YYYY-MM-DD HH:MM:SS [LEVEL] message

示例：

2026-08-09 14:32:08 [ERROR] Database connection timeout

支持的日志级别：

TRACE、DEBUG、INFO、WARNING、ERROR、CRITICAL

## 错误处理

- 文件不存在：输出错误并返回非零退出码
- 文件无法读取：输出错误并返回非零退出码
- 单行格式损坏：记录 malformed 数量，继续扫描
- 参数无效：输出参数说明并返回非零退出码

## v0.1.0 非目标

- 不支持目录扫描
- 不支持多线程
- 不支持正则表达式
- 不支持自定义日志格式
- 不支持实时日志跟踪

默认构建只验证框架接口可以被包含和链接，不验证任何业务行为。
