## 阅读代码
整体的架构已经串好了, 这章拓展功能, 就跟着书走了, 将书中展示的代码注释好:

```cpp
llvm::Type *CGModule::convertType(TypeDeclaration *Ty) {
  // 检查类型是否已经在类型缓存中，如果存在直接返回缓存的类型
  if (llvm::Type *T = TypeCache[Ty])
    return T;

  // 处理内建类型（PervasiveTypeDeclaration）
  if (llvm::isa<PervasiveTypeDeclaration>(Ty)) {
    // 如果类型名称为 "INTEGER"，返回 64 位整数类型
    if (Ty->getName() == "INTEGER")
      return Int64Ty;
    // 如果类型名称为 "BOOLEAN"，返回 1 位整数类型
    if (Ty->getName() == "BOOLEAN")
      return Int1Ty;
  } 
  // 处理类型别名（AliasTypeDeclaration）
  else if (auto *AliasTy = llvm::dyn_cast<AliasTypeDeclaration>(Ty)) {
    // 转换别名指向的实际类型，并将其存入类型缓存
    llvm::Type *T = convertType(AliasTy->getType());
    return TypeCache[Ty] = T;
  } 
  // 处理数组类型（ArrayTypeDeclaration）
  else if (auto *ArrayTy = llvm::dyn_cast<ArrayTypeDeclaration>(Ty)) {
    // 转换数组的元素类型
    llvm::Type *Component = convertType(ArrayTy->getType());
    // 获取数组的大小（元素数量）
    // 语义分析保证 Nums 是一个整数常量表达式，这里简化处理，直接期望是一个整数字面量
    // TODO：评估常量表达式
    Expr *Nums = ArrayTy->getNums();
    assert(llvm::cast<IntegerLiteral>(Nums) && "Expected an integer literal");
    uint64_t NumElements = llvm::cast<IntegerLiteral>(Nums)->getValue().getZExtValue();
    // 创建 LLVM 数组类型，并存入类型缓存
    llvm::Type *T = llvm::ArrayType::get(Component, NumElements);
    return TypeCache[Ty] = T;
  } 
  // 处理记录类型（结构体）（RecordTypeDeclaration）
  else if (auto *RecordTy = llvm::dyn_cast<RecordTypeDeclaration>(Ty)) {
    // 创建一个小型向量来存储记录类型（结构体）的字段类型
    llvm::SmallVector<llvm::Type *, 4> Elements;
    // 遍历记录类型（结构体）的字段，转换每个字段的类型
    for (const auto &F : RecordTy->getFields()) {
      Elements.push_back(convertType(F.getType()));
    }
    // 创建 LLVM 结构体类型，并存入类型缓存
    llvm::Type *T = llvm::StructType::create(Elements, RecordTy->getName(), false);
    return TypeCache[Ty] = T;
  }
  // 如果类型无法识别，报告致命错误
  llvm::report_fatal_error("Unsupported type");
}
```

```cpp
void CGProcedure::run(ProcedureDeclaration *Proc) {
  // 设置当前处理的程序声明
  this->Proc = Proc;
  // 创建函数类型
  Fty = createFunctionType(Proc);
  // 创建 LLVM 函数
  Fn = createFunction(Proc, Fty);

  // 创建一个基本块，命名为 "entry"，作为函数的入口
  llvm::BasicBlock *BB = createBasicBlock("entry");
  // 设置当前基本块为 "entry"
  setCurr(BB);

  // 遍历函数参数，为每个参数创建映射并初始化局部变量
  size_t Idx = 0;
  for (auto I = Fn->arg_begin(), E = Fn->arg_end(); I != E; ++I, ++Idx) {
    llvm::Argument *Arg = I; // 获取当前参数
    FormalParameterDeclaration *FP = Proc->getFormalParams()[Idx]; // 获取对应的形参声明
    // 为形参创建到 LLVM 参数的映射，特别针对 VAR 参数
    FormalParams[FP] = Arg;
    // 将参数值写入局部变量
    writeLocalVariable(Curr, FP, Arg);
  }

  // 遍历过程声明中的变量声明
  for (auto *D : Proc->getDecls()) {
    if (auto *Var = llvm::dyn_cast<VariableDeclaration>(D)) { // 如果是变量声明
      llvm::Type *Ty = mapType(Var); // 获取变量的 LLVM 类型
      if (Ty->isAggregateType()) { // 如果是聚合类型（如结构体、数组等）
        // 为变量分配栈内存
        llvm::Value *Val = Builder.CreateAlloca(Ty);
        // 将分配的内存地址写入局部变量
        writeLocalVariable(Curr, Var, Val);
      }
    }
  }

  // 获取过程声明中的语句块
  auto Block = Proc->getStmts();
  // 发射（生成代码）语句块
  emit(Proc->getStmts());
  // 如果当前基本块没有终止指令，则添加一个返回指令
  if (!Curr->getTerminator()) {
    Builder.CreateRetVoid();
  }
  // 封闭当前基本块
  sealBlock(Curr);
}
```

```cpp
llvm::Value *CGProcedure::emitExpr(Expr *E) {
  // 处理二元表达式（如加法、减法等）
  if (auto *Infix = llvm::dyn_cast<InfixExpression>(E)) {
    return emitInfixExpr(Infix);
  } 
  // 处理一元表达式（如取反、取地址等）
  else if (auto *Prefix = llvm::dyn_cast<PrefixExpression>(E)) {
    return emitPrefixExpr(Prefix);
  } 
  // 处理变量访问（包括数组、记录等复杂类型）
  else if (auto *Var = llvm::dyn_cast<Designator>(E)) {
    // 获取变量声明
    auto *Decl = Var->getDecl();
    // 读取变量的值
    llvm::Value *Val = readVariable(Curr, Decl);

    // 获取选择器列表（例如数组下标、字段选择等）
    auto &Selectors = Var->getSelectors();
    for (auto I = Selectors.begin(), E = Selectors.end(); I != E; /* no increment */) {
      // 处理数组下标选择器
      if (auto *IdxSel = llvm::dyn_cast<IndexSelector>(*I)) {
        llvm::SmallVector<llvm::Value *, 4> IdxList; // 存储下标值
        while (I != E) {
          if (auto *Sel = llvm::dyn_cast<IndexSelector>(*I)) {
            // 递归生成下标表达式的值
            IdxList.push_back(emitExpr(Sel->getIndex()));
            ++I;
          } else
            break;
        }
        // 使用 GEP 指令生成数组元素的地址
        Val = Builder.CreateInBoundsGEP(Val->getType(), Val, IdxList);
        // 加载数组元素的值
        Val = Builder.CreateLoad(Val->getType(), Val);
      } 
      // 处理记录字段选择器
      else if (auto *FieldSel = llvm::dyn_cast<FieldSelector>(*I)) {
        llvm::SmallVector<llvm::Value *, 4> IdxList; // 存储字段索引
        while (I != E) {
          if (auto *Sel = llvm::dyn_cast<FieldSelector>(*I)) {
            // 创建字段索引的常量
            llvm::Value *V = llvm::ConstantInt::get(CGM.Int64Ty, Sel->getIndex());
            IdxList.push_back(V);
            ++I;
          } else
            break;
        }
        // 使用 GEP 指令生成字段的地址
        Val = Builder.CreateInBoundsGEP(Val->getType(), Val, IdxList);
        // 加载字段的值
        Val = Builder.CreateLoad(Val->getType(), Val);
      } 
      // 处理解引用选择器
      else if (auto *DerefSel = llvm::dyn_cast<DereferenceSelector>(*I)) {
        // 加载解引用后的值
        Val = Builder.CreateLoad(Val->getType(), Val);
        ++I;
      } 
      // 如果遇到不支持的选择器类型，报告错误
      else {
        llvm::report_fatal_error("Unsupported selector");
      }
    }

    return Val;
  } 
  // 处理常量访问
  else if (auto *Const = llvm::dyn_cast<ConstantAccess>(E)) {
    // 递归生成常量表达式的值
    return emitExpr(Const->getDecl()->getExpr());
  } 
  // 处理整数字面量
  else if (auto *IntLit = llvm::dyn_cast<IntegerLiteral>(E)) {
    // 创建对应的 LLVM 整数常量
    return llvm::ConstantInt::get(CGM.Int64Ty, IntLit->getValue());
  } 
  // 处理布尔字面量
  else if (auto *BoolLit = llvm::dyn_cast<BooleanLiteral>(E)) {
    // 创建对应的 LLVM 布尔常量
    return llvm::ConstantInt::get(CGM.Int1Ty, BoolLit->getValue());
  }
  // 如果遇到不支持的表达式类型，报告错误
  llvm::report_fatal_error("Unsupported expression");
}
```

为类和虚函数创建 ir 这章要好好看看, 在编译的过程中其实只做了两件事: 将高级语言的抽象用低级类汇编语言实现, 优化。