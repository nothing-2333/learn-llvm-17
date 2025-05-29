## 偷懒了, 没有下载工具, 直接看一波代码吧
主要是围绕这段话来的
```
新的IconvChecker类需要处理四种类型的事件:

PostCall，发生在函数调用之后。调用iconv_open()函数之后，检索返回值的符号，并记住其处于“打开”状态。

PreCall，发生在函数调用之前。调用iconv_close()函数之前，检查描述符的符号是否处于“打开”状态。若没有，则已经为描述符调用了iconv_close()函数，并且已经检测到对该函数的两次调用。

DeadSymbols，在清理未使用的符号时发生。我们检查描述符中未使用的符号是否仍处于“打开”状态。是的话，则检测到缺少对iconv_close()的调用，这是一个资源泄漏。

PointerEscape，当分析器无法再跟踪符号时调用该函数。这种情况下，因为不能再推断描述符是否已关闭，所以需要从状态中删除符号。
```

```cpp
// 检查函数调用后的状态
void IconvChecker::checkPostCall(const CallEvent &Call, CheckerContext &C) const {
  if (!Call.isGlobalCFunction())  // 如果不是全局 C 函数，直接返回
    return;
  if (!IconvOpenFn.matches(Call))  // 如果不是 iconv_open 函数，直接返回
    return;
  if (SymbolRef Handle = Call.getReturnValue().getAsSymbol()) {  // 获取返回值符号
    ProgramStateRef State = C.getState();  // 获取当前状态
    State = State->set<IconvStateMap>(  // 设置状态为打开
        Handle, IconvState::getOpened());
    C.addTransition(State);  // 添加状态转换
  }
}

// 检查函数调用前的状态
void IconvChecker::checkPreCall(const CallEvent &Call, CheckerContext &C) const {
  if (!Call.isGlobalCFunction()) {  // 如果不是全局 C 函数，直接返回
    return;
  }
  if (!IconvCloseFn.matches(Call)) {  // 如果不是 iconv_close 函数，直接返回
    return;
  }
  if (SymbolRef Handle = Call.getArgSVal(0).getAsSymbol()) {  // 获取第一个参数符号
    ProgramStateRef State = C.getState();  // 获取当前状态
    if (const IconvState *St = State->get<IconvStateMap>(Handle)) {  // 获取状态
      if (!St->isOpen()) {  // 如果状态不是打开
        if (ExplodedNode *N = C.generateErrorNode()) {  // 生成错误节点
          report(Handle, *DoubleCloseBugType,  // 报告错误
                 "Closing a previous closed iconv descriptor", C, N, Call.getSourceRange());
        }
        return;
      }
    }

    State = State->set<IconvStateMap>(  // 设置状态为关闭
        Handle, IconvState::getClosed());
    C.addTransition(State);  // 添加状态转换
  }
}

// 检查死亡符号
void IconvChecker::checkDeadSymbols(SymbolReaper &SymReaper, CheckerContext &C) const {
  ProgramStateRef State = C.getState();  // 获取当前状态
  SmallVector<SymbolRef, 8> LeakedSyms;  // 用于存储泄漏的符号
  for (auto [Sym, St] : State->get<IconvStateMap>()) {  // 遍历状态映射
    if (SymReaper.isDead(Sym)) {  // 如果符号死亡
      if (St.isOpen()) {  // 如果状态是打开
        bool IsLeaked = true;
        if (const llvm::APSInt *Val = State->getConstraintManager().getSymVal(  // 获取符号值
            State, Sym))
          IsLeaked = Val->getExtValue() != -1;
        if (IsLeaked)
          LeakedSyms.push_back(Sym);  // 添加到泄漏符号列表
      }
      State = State->remove<IconvStateMap>(Sym);  // 从状态中移除符号
    }
  }

  if (ExplodedNode *N = C.generateNonFatalErrorNode(State)) {  // 生成非致命错误节点
    report(LeakedSyms, *LeakBugType,  // 报告泄漏错误
           "Opened iconv descriptor not closed", C, N);
  }
}

// 检查指针逃逸
ProgramStateRef IconvChecker::checkPointerEscape(ProgramStateRef State,
                                                 const InvalidatedSymbols &Escaped,
                                                 const CallEvent *Call,
                                                 PointerEscapeKind Kind) const {
  if (Kind == PSK_DirectEscapeOnCall) {  // 如果是直接逃逸
    if (IconvFn.matches(*Call) || IconvCloseFn.matches(*Call))  // 如果是 iconv 或 iconv_close 函数
      return State;
    if (Call->isInSystemHeader() || !Call->argumentsMayEscape())  // 如果在系统头文件中或参数不会逃逸
      return State;
  }

  for (SymbolRef Sym : Escaped)  // 遍历逃逸的符号
    State = State->remove<IconvStateMap>(Sym);  // 从状态中移除符号
  return State;
}
```