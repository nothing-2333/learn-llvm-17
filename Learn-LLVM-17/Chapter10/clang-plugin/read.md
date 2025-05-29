## 试一试
```bash
./build.sh
clang -fplugin=build/NamingPlugin.so example/naming.c -o naming
```

## 按套路走
注释主要逻辑代码
```cpp
// 重写 HandleTopLevelDecl 方法，处理顶层声明
bool HandleTopLevelDecl(DeclGroupRef DG) override {
    // 遍历 DeclGroupRef 中的每个声明
    for (DeclGroupRef::iterator I = DG.begin(), E = DG.end(); I != E; ++I) {
        const Decl *D = *I;  // 获取当前声明
        // 检查是否是 FunctionDecl 类型
        if (const FunctionDecl *FD = dyn_cast<FunctionDecl>(D)) {
            // 获取函数名
            std::string Name = FD->getNameInfo().getName().getAsString();
            // 断言函数名不为空
            assert(Name.length() > 0 && "Unexpected empty identifier");
            // 获取函数名的第一个字符
            char &First = Name.at(0);
            // 检查第一个字符是否不是小写字母
            if (!(First >= 'a' && First <= 'z')) {
                // 获取诊断引擎
                DiagnosticsEngine &Diag = CI.getDiagnostics();
                // 创建自定义诊断 ID
                unsigned ID = Diag.getCustomDiagID(
                    DiagnosticsEngine::Warning,
                    "Function name should start with lowercase letter");
                // 报告诊断信息
                Diag.Report(FD->getLocation(), ID);
            }
        }
    }
    return true;  // 返回 true 表示处理成功
}
```
