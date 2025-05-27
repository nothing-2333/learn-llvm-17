## 体验一下
```bash
./tinylang-tblgen --gen-tokens -o TokenFilter.inc Keyword.td 
```

## 注释一下 CMakeLists.txt 然后从 `TableGen.cpp` 开始
读一读这两个 `main` `Main` 函数就可以了, 为了减少脑细胞的消耗, 我做好了注释:
```cpp
// 主函数
int main(int argc, char **argv) {
  // 在出错时打印堆栈跟踪
  sys::PrintStackTraceOnErrorSignal(argv[0]);
  // 初始化 PrettyStackTraceProgram，用于美化堆栈跟踪
  PrettyStackTraceProgram X(argc, argv);
  // 解析命令行选项
  cl::ParseCommandLineOptions(argc, argv);

  // 确保 LLVM 的资源在退出时被正确清理
  llvm_shutdown_obj Y;

  // 调用 TableGenMain，将 Main 函数作为回调
  return TableGenMain(argv[0], &Main);
}
```
```cpp
// 主逻辑函数
bool Main(raw_ostream &OS, RecordKeeper &Records) {
  // 根据用户选择的操作类型执行不同的逻辑
  switch (Action) {
  case PrintRecords:
    OS << Records; // 无参数，输出所有内容
    break;
  case DumpJSON:
    EmitJSON(Records, OS); // 输出 JSON 格式
    break;
  case GenTokens:
    EmitTokensAndKeywordFilter(Records, OS); // 生成 Token 相关代码
    break;
  }

  return false; // 返回成功
}
```
解析命令行后 `--gen-tokens` 对应着 `EmitTokensAndKeywordFilter`
## `EmitTokensAndKeywordFilter` 函数
首先是 run 函数, 这是我们的第一站:
```cpp
void TokenAndKeywordFilterEmitter::run(raw_ostream &OS) {
  // 生成 Flags 代码
  Records.startTimer("Emit flags"); // 开始计时，用于性能分析
  emitFlagsFragment(OS);

  // 生成 TokenKind 代码
  Records.startTimer("Emit token kind"); // 开始计时
  emitTokenKind(OS);

  // 生成 Keyword 代码
  Records.startTimer("Emit keyword filter"); // 开始计时
  emitKeywordFilter(OS);
  Records.stopTimer(); // 停止计时
}
```
在之后的 `emitFlagsFragment` `emitTokenKind` `emitKeywordFilter` 函数中, llvm 为我们做了 TableGen 语言的前端解析, 我们要做的是利用这个生成自己的代码, 将字符串拼接好。
