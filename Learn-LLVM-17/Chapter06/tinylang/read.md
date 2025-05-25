## TBAA 元数据
围绕着 `createScalarTypeNode` `createStructTypeNode` `getTypeInfo` 三个函数实现了主要逻辑, `CGM.decorateInst` 添加注释, 比较简单就不在一一解释了, 自己跟一跟。重点讲一下 TBAA

### 结构
1. **标识字段（Identity Field）**：用于唯一标识当前类型。
2. **父节点字段（Parent Field）**：指向当前类型的父节点，用于构建类型树。
3. **偏移字段（Offset Field）**：表示当前类型在父类型中的偏移量。

### 示例
```llvm
!0 = !{!"m\83\826\06\00\00\00\00\A2\8Ef\7F\C9:n", !1, i64 0, !1, i64 8}
!1 = !{!"INTEGER", !2, i64 0}
!2 = !{!"Simple tinylang TBAA"}
```

- **`!0`**：表示 `int` 类型，其父节点是 `!2`。
  - !0：定义了一个 TBAA 元数据节点，表示 Person 结构体的第二个字段。
  - !"m\83\826\06\00\00\00\00\A2\8Ef\7F\C9:n"：标识字段的名称。
  - !1：指向字段的类型信息。
  - i64 0：字段在结构体中的偏移量（从结构体的起始位置开始）。
  - !1：再次指向字段的类型信息。
  - i64 8：字段在结构体中的偏移量（从上一个字段的结束位置开始）。
- **`!1`**：
  - !1：定义了一个 TBAA 元数据节点，表示字段的类型信息。
  - !"INTEGER"：字段的类型名称。
  - !2：指向根节点。
  - i64 0：字段在类型中的偏移量。
- **`!2`**：定义了一个 TBAA 根节点，表示整个 TBAA 系统的起点。!"Simple tinylang TBAA"：根节点的名称。

### 应用举例
```c
int foo(int* a, int* b) {
    *a = 42;
    *b = 20;
    return *a;
}
```
生成的 LLVM IR 如下：
```llvm
define i32 @foo(i32* %0, i32* %1) {
  store i32 42, i32* %0, align 4, !tbaa !1
  store i32 20, i32* %1, align 4, !tbaa !1
  %3 = load i32, i32* %0, align 4, !tbaa !1
  ret i32 %3
}
```

- **`load i32, i32* %0, align 4, !tbaa !1`**：表示加载的指针是 `!1` 类型, 以此类推
- **`!tbaa !1`**：表示这些指令使用了 TBAA 元数据。
- 如果编译器确定 `a` 和 `b` 不会别名（即它们指向不同的内存位置），则可以优化掉不必要的 `load` 指令，直接返回 `42`。

## 调试信息
可以分别尝试一下 `./tinylang -emit-llvm ./examples/Person.mod` `./tinylang -emit-llvm ./examples/Person.mod -g` 从 ir 开始:
```llvm
; 模块的源文件路径
source_filename = "./examples/Person.mod"

; 目标数据布局
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"

; 目标三元组
target triple = "x86_64-unknown-linux-gnu"

; 定义一个 Person 类型的结构体
%Person = type { i64, i64 }

; 定义一个函数 _t6Person3Set，接收一个 Person 类型的指针参数
define void @_t6Person3Set(ptr nocapture dereferenceable(16) %p) !dbg !2 {
entry:
  ; 调用 llvm.dbg.value 内置函数，记录变量 p 的调试信息
  call void @llvm.dbg.value(metadata ptr %p, metadata !8, metadata !DIExpression()), !dbg !9
  ; 获取 p 指向的 Person 结构体的第二个字段的地址
  %0 = getelementptr inbounds %Person, ptr %p, i32 0, i32 1
  ; 将值 18 存储到该字段
  store i64 18, ptr %0, align 8, !tbaa !10
  ; 返回
  ret void
}

; 声明 llvm.dbg.value 内置函数
; Function Attrs: nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare void @llvm.dbg.value(metadata, metadata, metadata) #0

; 定义函数属性
attributes #0 = { nocallback nofree nosync nounwind speculatable willreturn memory(none) }

; 调试信息的编译单元
!llvm.dbg.cu = !{!0}

; 编译单元的调试信息
!0 = distinct !DICompileUnit(language: DW_LANG_Modula2, file: !1, producer: "tinylang", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug)
; 文件的调试信息
!1 = !DIFile(filename: "Person.mod", directory: "/home/nothing/learn-llvm-17/Learn-LLVM-17/Chapter06/tinylang/./examples")
; 函数 Set 的调试信息
!2 = distinct !DISubprogram(name: "Set", linkageName: "_t6Person3Set", scope: !1, file: !1, line: 9, type: !3, scopeLine: 9, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !7)
; 子程序类型
!3 = !DISubroutineType(types: !4)
; 子程序类型参数
!4 = !{!5, !6}
; 返回类型（void）
!5 = !DIBasicType(tag: DW_TAG_unspecified_type, name: "void")
; 参数类型（引用类型）
!6 = !DIDerivedType(tag: DW_TAG_reference_type, baseType: null, size: 1024, align: 8)
; 空的保留节点
!7 = !{}
; 参数变量 p 的调试信息
!8 = !DILocalVariable(name: "p", arg: 1, scope: !2, file: !1, line: 9)
; 调试位置信息
!9 = !DILocation(line: 9, column: 19, scope: !2)
; 类型别名信息
!10 = !{!"=\DD\D3`\05\00\00\00\A7c\83\22\F6O\F8\C1", !11, i64 0, !11, i64 8}
; 类型别名
!11 = !{!"INTEGER", !12, i64 0}
; 类型别名的描述
!12 = !{!"Simple tinylang TBAA"}
```
添加调试信息就是要生成一份这样的 ir, debug 版本的软件一般要比 release 版本大很多, 是因为 debug 不仅不会优化, 还增加了一些额外的信息与调用。让我们先看懂这份 ir 再去阅读源码。

### !dbg 的作用
用于附加调试位置信息的元数据标记。它将 LLVM IR 中的指令与源代码的位置（文件名、行号、列号）关联起来

### metadata 的作用
用于存储调试信息、属性信息等。调试信息（如变量名、类型、文件路径等）通常以 metadata 的形式嵌入到 LLVM IR 中
- metadata ptr %p: 表示变量的值。
- metadata !8: 表示变量的调试信息，例如变量的名称、类型、作用域等。
- metadata !DIExpression(): 表示变量的表达式信息，用于描述变量的复杂表达式或位置。

### 为什么需要 @llvm.dbg.value 而不是直接使用 !dbg 和 metadata
在运行时，变量的值可能会发生变化。@llvm.dbg.value 用于在特定的指令位置记录变量的当前值。如果没有这个调用，调试工具将无法知道变量的值。虽然 !dbg 和 metadata 可以记录调试信息，但它们本身并不足以将变量的值与调试信息关联起来。@llvm.dbg.value 的作用是显式地将变量的值与调试信息绑定，以便调试工具能够在运行时正确地显示变量的值。

### 再去看 `lib/CodeGen/CGDebugInfo.cpp` 源码就比较清楚了
- `CGDebugInfo::CGDebugInfo`: 初始化
- `CGDebugInfo::getScope`, `CGDebugInfo::openScope`, `CGDebugInfo::closeScope`: 维护作用域, `ScopeStack` 来管理一个个作用域来处理嵌套作用域的情况。
- `getArrayType` `getAliasType` `emitGlobalVariable`...其他函数: 利用 llvm 的 api 添加调试信息, 以及一些辅助函数。