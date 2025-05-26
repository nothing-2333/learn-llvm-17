## 总结一下编写 LLVM IR 插件的核心套路
- 继承 PassInfoMixin 模板类, 实现 run 方法: 在 run 方法中实现插件的核心逻辑
- 回调函数注册: 通过 PassBuilder 注册两种回调方式, 在 `RegisterCB` 函数中实现了注册逻辑

## 编译与部署​
更改 llvm-project, 增量编译后, 来集成插件, ppprofiler-intree 与 ppprofiler.diff 文件夹中是修改 llvm-project 源码的点。

## ​​动态加载与静态链接
```cpp
llvm::PassPluginLibraryInfo getPPProfilerPluginInfo() {
  return {LLVM_PLUGIN_API_VERSION, "PPProfiler", "v0.1",
          RegisterCB};
}

#ifndef LLVM_PPPROFILER_LINK_INTO_TOOLS
extern "C" LLVM_ATTRIBUTE_WEAK ::llvm::PassPluginLibraryInfo
llvmGetPassPluginInfo() {
  return getPPProfilerPluginInfo();
}
#endif
```
#ifndef LLVM_PPPROFILER_LINK_INTO_TOOLS 表示当未定义该宏时，才会编译后续代码块。此时为​​动态加载​​（默认）: 插件编译为 .so 文件，通过 -fpass-plugin 参数加载;而静态链接​​: 通过定义宏 LLVM_PPPROFILER_LINK_INTO_TOOLS，将插件直接编译进 LLVM 工具链（如 opt、clang），无需动态加载

## 使用 pass
### opt
仅修改了 ​​Transforms/PPProfiler 目录下的代码或 CMakeLists.txt​​，且未调整全局 CMake 配置选项, 直接增量编译即可, 千万不要全部构建一遍! 千万不要全部构建一遍! 千万不要全部构建一遍!
```bash
ninja -C build install -j6
```
这样会将 PPProfiler.so 放在<install directory>/lib 目录下, 比如我的
```bash
ls /usr/local/lib | grep "PPProfiler"
# PPProfiler.so
```

#### 使用 pass 
```bash
## 编译出 .bc 文件
clang -S -emit-llvm -O1 hello.c
opt --load-pass-plugin=PPProfiler.so --passes="ppprofiler" --stats example/hello.ll -o example/hello_inst.bc

## 反汇编 .bc 文件
llvm-dis example/hello_inst.bc -o -

## 运行 
clang example/hello_inst.bc runtime/runtime.c
./a.out
```