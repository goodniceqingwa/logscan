# logscan

这是一个使用 C++17 开发的本地日志分析工具。当前已经支持按批读取普通文件、解析固定格式日志，以及按精确级别和区分大小写的正文子串筛选结构化日志；线程调度、命令行和输出流水线仍在开发中。

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

`--level` 使用精确级别匹配，`--contains` 使用区分大小写的正文子串匹配；两个条件同时设置时使用 AND。

## 支持的日志格式

YYYY-MM-DD HH:MM:SS [LEVEL] message

示例：

2026-08-09 14:32:08 [ERROR] Database connection timeout

支持的日志级别：

TRACE、DEBUG、INFO、WARNING、ERROR、CRITICAL

时间戳按运行机器的本地时区解释，年份范围为 1970～9999。

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

默认构建会运行公共 API、文件批量读取、固定格式解析和筛选分析测试。
