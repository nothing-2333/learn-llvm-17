#include "CodeGen.h"
#include "AST.h"
#include "llvm/ADT/StringMap.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/Support/raw_ostream.h"
#include <llvm/ADT/StringRef.h>
#include <llvm/ADT/Twine.h>
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/Constant.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/GlobalValue.h>
#include <llvm/IR/GlobalVariable.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/Value.h>

using namespace llvm;

namespace {
    class ToIRVisitor : public ASTVisitor
    {
        Module *module;
        IRBuilder<> builder;
        Type *void_ty;
        Type *int32_ty;
        PointerType *ptr_ty;
        Constant *int32_zero;
        Value *value;
        StringMap<Value *> name_map;

    public:
        ToIRVisitor(Module *module) : module(module), builder(module->getContext())
        {
            void_ty = Type::getVoidTy(module->getContext());
            int32_ty = Type::getInt32Ty(module->getContext());
            ptr_ty = PointerType::getUnqual(module->getContext());
            int32_zero = ConstantInt::get(int32_ty, 0, true);
        }

        void run(AST *tree)
        {
            // 在 LLVM IR 中定义 main() 函数
            FunctionType *main_fn_ty = FunctionType::get(int32_ty, { int32_ty, ptr_ty }, false);
            Function *main_fn = Function::Create(main_fn_ty, GlobalValue::ExternalLinkage, "main", module);
            // 然后用 entry 标签创建BB基本块，并将其添加到IR构建器
            BasicBlock *bb = BasicBlock::Create(module->getContext(), "entry", main_fn);
            builder.SetInsertPoint(bb);
            // 遍历树
            tree->accept(*this);
            // 调用 calc_write() 函数输出计算值
            FunctionType *calc_write_fn_ty = FunctionType::get(void_ty, {int32_ty}, false);
            Function *calc_write_fn = Function::Create(calc_write_fn_ty, GlobalValue::ExternalLinkage,
                 "calc_write", module);
            builder.CreateCall(calc_write_fn_ty, calc_write_fn, {value});
            // 返回 0 结束
            builder.CreateRet(int32_zero);
        }

        virtual void visit(WithDecl &node) override
        {
            FunctionType *read_fn_ty = FunctionType::get(int32_ty, {ptr_ty}, false);
            Function *read_fn = Function::Create(read_fn_ty, GlobalValue::ExternalLinkage, "calc_read", module);
        
            // 对于每个变量，创建一个带有变量名的字符串
            for (auto begin = node.begin(), end = node.end(); begin != end; ++begin)
            {
                StringRef var = *begin;
                Constant *str_text = ConstantDataArray::getString(module->getContext(), var);
                GlobalVariable *str = new GlobalVariable(*module, str_text->getType(),
                    true, GlobalVariable::PrivateLinkage,
                    str_text, Twine(var).concat(".str"));
                // 创建调用 calc_read() 函数的 IR 代码，使用上一步中创建的字符串作为参数传递
                CallInst *call = builder.CreateCall(read_fn_ty, read_fn, {str});
                // 返回值存储在 name_map 映射中供以后使用
                name_map[var] = call;
            }

            // 继续遍历 ast
            node.get_expr()->accept(*this);
        }

        virtual void visit(Factor &node) override
        {
            if (node.get_kind() == Factor::Ident) value = name_map[node.get_val()];
            else 
            {
                int intval;
                node.get_val().getAsInteger(10, intval);
                value = ConstantInt::get(int32_ty, intval, true);
            }
        }

        virtual void visit(BinaryOp &node) override
        {
            node.get_left()->accept(*this);
            Value *left = value;
            node.get_right()->accept(*this);
            Value *right = value;
            switch (node.get_operator()) {
                case BinaryOp::Plus:
                    value = builder.CreateNSWAdd(left, right); 
                    break;
                case BinaryOp::Minus:
                    value = builder.CreateNSWSub(left, right); 
                    break;
                case BinaryOp::Mul:
                    value = builder.CreateNSWMul(left, right); 
                    break;
                case BinaryOp::Div:
                    value = builder.CreateSDiv(left, right); 
                    break;
            }
        }
    };
}

void CodeGen::compile(AST *tree)
{
    LLVMContext ctx;
    Module *module = new Module("calc.expr", ctx);
    ToIRVisitor to_ir(module);
    to_ir.run(tree);
    module->print(outs(), nullptr);
}