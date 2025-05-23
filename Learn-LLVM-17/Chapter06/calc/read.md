## 源码阅读

### 从这里看起
```cpp
case BinaryOp::Div:
    BasicBlock *TrueDest, *FalseDest;
    createICmpEq(Right, Int32Zero, TrueDest, FalseDest, "divbyzero",
                "notzero");
    Builder.SetInsertPoint(TrueDest);
    addThrow(42); // Arbitrary payload
    Builder.SetInsertPoint(FalseDest);
    V = Builder.CreateSDiv(Left, Right);
    break;
```
对应的 ir 为:
```llvm
  ; 判断输入值是否为 0
  %3 = icmp eq i32 %2, 0
  ; 根据输入值是否为 0，跳转到不同的分支
  br i1 %3, label %divbyzero, label %notzero

divbyzero:                                        ; preds = %entry
  ; 如果输入值为 0，分配异常对象内存
  %4 = call ptr @__cxa_allocate_exception(i64 4)
  ; 将异常值 42 存储到分配的内存中
  store i32 42, ptr %4, align 4
  ; 抛出异常
  invoke void @__cxa_throw(ptr %4, ptr @_ZTIi, ptr null)
          to label %unreachable unwind label %lpad

notzero:                                          ; preds = %entry
  ; 如果输入值不为 0，执行正常逻辑，计算 3 / 输入值
  %5 = sdiv i32 3, %2
  ; 调用 calc_write 函数输出结果
  call void @calc_write(i32 %5)
  ; 返回 0，表示程序正常结束
  ret i32 0
```
输入 0 后, 走到 `invoke void @__cxa_throw(ptr %4, ptr @_ZTIi, ptr null) to label %unreachable unwind label %lpad` __cxa_throw 是 c++ 的 abi, 正常的话 __cxa_throw 函数会抛出异常走到 %lpad 基本块, 如果 __cxa_throw 内部发生了错误(概率很小) 走到 %unreachable 基本块。

### 再来解析一下 `@__cxa_throw(ptr %4, ptr @_ZTIi, ptr null)` 函数
函数原型:
```cpp
void __cxa_throw(void* thrown_object, std::type_info* tinfo, void (*destructor)(void*));
```
- thrown_object: 这是异常对象的内存地址。在这个案例中是 ptr %4 存储了一个整数值 42
- tinfo: 类型信息用于在异常处理过程中识别异常的类型, 在 catch 块中，运行时会检查捕获的异常对象的类型信息是否匹配, 从而决定是否捕获。
- destructor: 这是一个指向异常对象析构函数的指针。如果异常对象需要在销毁时执行清理操作，可以在这里传入析构函数的地址。

### 再结合 `addThrow` `addLandingPad` 函数看生成的 ir
源码太长我就不贴出来了, 只贴出 ir:
```llvm
lpad:                                             ; preds = %divbyzero
  ; 着陆垫块，用于捕获异常
  %exc = landingpad { ptr, i32 }
          catch ptr @_ZTIi
  ; 提取选择器值
  %exc.sel = extractvalue { ptr, i32 } %exc, 1
  ; 获取异常类型的 ID
  %6 = call i32 @llvm.eh.typeid.for(ptr @_ZTIi)
  ; 判断选择器值是否与异常类型 ID 匹配
  %7 = icmp eq i32 %exc.sel, %6
  ; 根据匹配结果跳转到不同的分支
  br i1 %7, label %match, label %resume

match:                                            ; preds = %lpad
  ; 如果匹配成功，提取异常对象指针
  %exc.ptr = extractvalue { ptr, i32 } %exc, 0
  ; 调用 __cxa_begin_catch 开始捕获异常
  %8 = call ptr @__cxa_begin_catch(ptr %exc.ptr)
  ; 输出错误信息 "Divide by zero!"
  %9 = call i32 @puts(ptr @msg)
  ; 调用 __cxa_end_catch 结束捕获异常
  call void @__cxa_end_catch()
  ; 返回 0，表示异常被捕获并处理
  ret i32 0

resume:                                           ; preds = %lpad
  ; 如果不匹配，继续传播异常
  resume { ptr, i32 } %exc
```

---
#### %exc = landingpad { ptr, i32 } catch ptr @_ZTIi
它定义了异常被捕获时的处理逻辑, 有点像类在实例化。

- 结构体类型 { ptr, i32 }
  - ptr：异常对象的指针。
  - i32：选择器值（selector value），用于标识异常的类型。
- catch ptr @_ZTIi: 表示捕获类型为 int 的异常。
- %exc: landingpad 指令的结果，存储了一个包含异常对象指针和选择器值的结构体。

#### extractvalue
LLVM IR 中用于从结构体或数组中提取特定字段的指令

#### @llvm.eh.typeid.for
是一个 LLVM 内置函数，用于获取异常类型的唯一标识符（ID）。这个 ID 用于在异常处理过程中判断捕获的异常是否与预期的异常类型匹配。

#### @llvm.eh.typeid.for 用于获取当前异常的id，为什么还要传入 ptr @_ZTIi
ptr @_ZTIi 是 C++ 中 int 类型的类型信息（std::type_info）的指针。通过这个指针，@llvm.eh.typeid.for 可以确定异常的具体类型。

---

剩下的就是根据是否匹配做输出或者继续传播异常。

### 完整 llvm ir `./build/src/calc "with a: 3/a" > tmp`, 做出注释
```llvm
; ModuleID = 'calc.expr'
source_filename = "calc.expr"

; 全局变量，存储字符串 "a"，用于 calc_read 函数
@a.str = private unnamed_addr constant [2 x i8] c"a\00", align 1

; 外部声明的异常类型信息，用于 __cxa_throw
@_ZTIi = external constant ptr

; 全局变量，存储错误信息 "Divide by zero!"，用于异常捕获时输出
@msg = private unnamed_addr constant [16 x i8] c"Divide by zero!\00", align 1

; 主函数 main，定义了异常处理的逻辑
define i32 @main(i32 %0, ptr %1) personality ptr @__gxx_personality_v0 {
entry:
  ; 调用 calc_read 函数，读取输入值
  %2 = call i32 @calc_read(ptr @a.str)
  ; 判断输入值是否为 0
  %3 = icmp eq i32 %2, 0
  ; 根据输入值是否为 0，跳转到不同的分支
  br i1 %3, label %divbyzero, label %notzero

divbyzero:                                        ; preds = %entry
  ; 如果输入值为 0，分配异常对象内存
  %4 = call ptr @__cxa_allocate_exception(i64 4)
  ; 将异常值 42 存储到分配的内存中
  store i32 42, ptr %4, align 4
  ; 抛出异常
  invoke void @__cxa_throw(ptr %4, ptr @_ZTIi, ptr null)
          to label %unreachable unwind label %lpad

notzero:                                          ; preds = %entry
  ; 如果输入值不为 0，执行正常逻辑，计算 3 / 输入值
  %5 = sdiv i32 3, %2
  ; 调用 calc_write 函数输出结果
  call void @calc_write(i32 %5)
  ; 返回 0，表示程序正常结束
  ret i32 0

lpad:                                             ; preds = %divbyzero
  ; 着陆垫块，用于捕获异常
  %exc = landingpad { ptr, i32 }
          catch ptr @_ZTIi
  ; 提取选择器值
  %exc.sel = extractvalue { ptr, i32 } %exc, 1
  ; 获取异常类型的 ID
  %6 = call i32 @llvm.eh.typeid.for(ptr @_ZTIi)
  ; 判断选择器值是否与异常类型 ID 匹配
  %7 = icmp eq i32 %exc.sel, %6
  ; 根据匹配结果跳转到不同的分支
  br i1 %7, label %match, label %resume

match:                                            ; preds = %lpad
  ; 如果匹配成功，提取异常对象指针
  %exc.ptr = extractvalue { ptr, i32 } %exc, 0
  ; 调用 __cxa_begin_catch 开始捕获异常
  %8 = call ptr @__cxa_begin_catch(ptr %exc.ptr)
  ; 输出错误信息 "Divide by zero!"
  %9 = call i32 @puts(ptr @msg)
  ; 调用 __cxa_end_catch 结束捕获异常
  call void @__cxa_end_catch()
  ; 返回 0，表示异常被捕获并处理
  ret i32 0

resume:                                           ; preds = %lpad
  ; 如果不匹配，继续传播异常
  resume { ptr, i32 } %exc

unreachable:                                      ; preds = %divbyzero
  ; 不可达块，表示异常未被捕获时的行为
  unreachable
}

; 声明 calc_read 函数，用于读取输入值
declare i32 @calc_read(ptr)

; 声明 __cxa_allocate_exception 函数，用于分配异常对象内存
declare ptr @__cxa_allocate_exception(i64)

; 声明 __cxa_throw 函数，用于抛出异常
declare void @__cxa_throw(ptr, ptr, ptr)

; 声明异常处理的个性函数
declare i32 @__gxx_personality_v0(...)

; 声明 llvm.eh.typeid.for 函数，用于获取异常类型的 ID
; Function Attrs: nounwind memory(none)
declare i32 @llvm.eh.typeid.for(ptr) #0

; 声明 __cxa_begin_catch 函数，用于开始捕获异常
declare ptr @__cxa_begin_catch(ptr)

; 声明 __cxa_end_catch 函数，用于结束捕获异常
declare void @__cxa_end_catch()

; 声明 puts 函数，用于输出字符串
declare i32 @puts(ptr)

; 声明 calc_write 函数，用于输出结果
declare void @calc_write(i32)

; 定义函数属性，表示函数不会抛出异常且不会访问内存
attributes #0 = { nounwind memory(none) }
```