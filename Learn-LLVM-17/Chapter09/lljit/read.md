## 让我们看看在第二章的基础上改了什么, 从 Calc.cpp 开始
注释一下这个 while, 这是整个项目的脉络。
```cpp
while (true) {
    // 提示用户输入
    outs() << "JIT calc > ";
    std::string calcExp;
    std::getline(std::cin, calcExp); // 读取用户输入

    // 创建新的上下文和模块
    std::unique_ptr<LLVMContext> Ctx = std::make_unique<LLVMContext>();
    std::unique_ptr<Module> M = std::make_unique<Module>("JIT calc.expr", *Ctx);
    M->setDataLayout(JIT->getDataLayout()); // 设置模块的数据布局

    // 声明代码生成器
    CodeGen CodeGenerator;

    // 创建词法分析器并获取下一个词法单元
    Lexer Lex(calcExp);
    Token::TokenKind CalcTok = Lex.peek();
    if (CalcTok == Token::KW_def) { // 如果用户定义了一个新函数
        Parser Parser(Lex); // 创建解析器
        AST *Tree = Parser.parse(); // 解析用户输入
        if (!Tree || Parser.hasError()) { // 如果解析失败
        llvm::errs() << "Syntax errors occured\n";
        return 1;
        }
        Sema Semantic; // 创建语义分析器
        if (Semantic.semantic(Tree, JITtedFunctions)) { // 如果语义分析失败
        llvm::errs() << "Semantic errors occured\n";
        return 1;
        }
        // 生成 IR 代码
        CodeGenerator.compileToIR(Tree, M.get(), JITtedFunctions);
        // 将模块添加到 JIT 中
        ExitOnErr(JIT->addIRModule(ThreadSafeModule(std::move(M), std::move(Ctx))));
    } else if (calcExp.find("quit") != std::string::npos) { // 如果用户输入了 "quit"
        outs() << "Quitting the JIT calc program.\n";
        break;
    } else if (CalcTok == Token::ident) { // 如果用户输入了一个标识符
        outs() << "Attempting to evaluate expression:\n";
        Parser Parser(Lex); // 创建解析器
        AST *Tree = Parser.parse(); // 解析用户输入
        if (!Tree || Parser.hasError()) { // 如果解析失败
            llvm::errs() << "Syntax errors occured\n";
            return 1;
        }
        Sema Semantic; // 创建语义分析器
        if (Semantic.semantic(Tree, JITtedFunctions)) { // 如果语义分析失败
            llvm::errs() << "Semantic errors occured\n";
            return 1;
        }
        llvm::StringRef FuncCallName = Tree->getFnName(); // 获取函数名称
        // 准备调用 JIT 编译的函数
        CodeGenerator.prepareCalculationCallFunc(Tree, M.get(), FuncCallName, JITtedFunctions);
        // 创建资源跟踪器
        auto RT = JIT->getMainJITDylib().createResourceTracker();
        auto TSM = ThreadSafeModule(std::move(M), std::move(Ctx));
        // 将模块添加到 JIT 中
        ExitOnErr(JIT->addIRModule(RT, std::move(TSM)));
        // 查找 JIT 编译的函数
        auto CalcExprCall = ExitOnErr(JIT->lookup("calc_expr_func"));
        // 获取函数地址并转换为函数指针
        int (*UserFnCall)() = CalcExprCall.toPtr<int (*)()>();
        // 调用 JIT 编译的函数并输出结果
        outs() << "User defined function evaluated to: " << UserFnCall() << "\n";
        // 释放资源
        ExitOnErr(RT->remove());
    }
}
```
顺着书中的介绍, 事实上我们只要关注这两个重载函数就可以了:
```cpp
// 虚函数，用于访问 DefDecl 节点（函数定义）
virtual void visit(DefDecl &Node) override {
    // 获取函数名称
    llvm::StringRef FnName = Node.getFnName();
    // 获取函数参数列表
    llvm::SmallVector<llvm::StringRef, 8> FunctionVars = Node.getVars();
    // 将函数名称和参数数量添加到 JIT 编译的函数列表中
    (JITtedFunctionsMap)[FnName] = FunctionVars.size();

    // 创建用户定义的函数
    Function *DefFunc = genUserDefinedFunction(FnName);
    if (!DefFunc) {
        llvm::errs() << "Error occurred when generating user defined function!\n";
        return;
    }

    // 创建一个新的基本块，用于插入用户定义的函数
    BasicBlock *BB = BasicBlock::Create(M->getContext(), "entry", DefFunc);
    Builder.SetInsertPoint(BB);

    // 为所有函数参数设置名称
    unsigned FIdx = 0;
    for (auto &FArg : DefFunc->args()) {
        nameMap[FunctionVars[FIdx]] = &FArg; // 将参数名称映射到参数对象
        FArg.setName(FunctionVars[FIdx++]); // 设置参数名称
    }

    // 生成参数之间的二元运算
    Node.getExpr()->accept(*this);
};

// 虚函数，用于访问 FuncCallFromDef 节点（函数调用）
virtual void visit(FuncCallFromDef &Node) override {
    llvm::StringRef CalcExprFunName = "calc_expr_func"; // 临时函数名称
    // 准备临时函数的签名，该函数将调用用户定义的函数
    FunctionType *CalcExprFunTy = FunctionType::get(Int32Ty, {}, false);
    Function *CalcExprFun = Function::Create(
        CalcExprFunTy, GlobalValue::ExternalLinkage, CalcExprFunName, M);

    BasicBlock *BB = BasicBlock::Create(M->getContext(), "entry", CalcExprFun);
    Builder.SetInsertPoint(BB);

    // 获取原始函数定义
    llvm::StringRef CalleeFnName = Node.getFnName();
    Function *CalleeFn = genUserDefinedFunction(CalleeFnName);
    if (!CalleeFn) {
        llvm::errs() << "Cannot retrieve the original callee function!\n";
        return;
    }

    // 设置函数调用的参数
    auto CalleeFnVars = Node.getArgs();
    llvm::SmallVector<Value *> IntParams;
    for (unsigned i = 0, end = CalleeFnVars.size(); i != end; ++i) {
        int ArgsToIntType;
        CalleeFnVars[i].getAsInteger(10, ArgsToIntType); // 将参数转换为整数
        Value *IntParam = ConstantInt::get(Int32Ty, ArgsToIntType, true); // 创建整数常量
        IntParams.push_back(IntParam); // 添加到参数列表
    }

    // 创建对用户定义函数的调用
    Value *Res = Builder.CreateCall(CalleeFn, IntParams, "calc_expr_res");
    Builder.CreateRet(Res); // 返回调用结果
};
```
我们只要生成 ir 剩下的交给 JIT