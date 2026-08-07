# logscan

这是一个并行日志分析器的 C++17 框架骨架。当前只定义扩展点和数据契约，不提供读取、解析、分析、线程调度或输出功能。

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

默认构建只验证框架接口可以被包含和链接，不验证任何业务行为。
