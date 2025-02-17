#include "ast.h"

#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/IRBuilder.h"

FunctionDecl::FunctionDecl(const std::string name) : name(name) {}

llvm::Value *FunctionDecl::codegen(llvm::IRBuilder<> &builder, llvm::LLVMContext &context, llvm::Module &module)
{
    llvm::FunctionType *funcType = llvm::FunctionType::get(llvm::Type::getVoidTy(context), false);
    llvm::Function *func = llvm::Function::Create(funcType, llvm::Function::ExternalLinkage, name, module);
    llvm::BasicBlock *block = llvm::BasicBlock::Create(context, "entry", func);
    builder.SetInsertPoint(block);

    for (auto &stmt : body)
    {
        stmt->codegen(builder, context, module);
    }

    builder.CreateRetVoid();

    return func;
}

FunctionCall::FunctionCall(const std::string &name) : name(name) {}
llvm::Value *FunctionCall::codegen(llvm::IRBuilder<> &builder, llvm::LLVMContext &context, llvm::Module &module)
{

    llvm::FunctionType *printfType = llvm::FunctionType::get(
        llvm::Type::getInt32Ty(context),                  // Return type (int)
        {llvm::Type::getInt8Ty(context)->getPointerTo()}, // First argument (char*)
        true                                              // Variadic function
    );

    // Create the function declaration
    llvm::Function::Create(
        printfType,
        llvm::Function::ExternalLinkage, // External linkage
        "printf",                        // Function name
        &module                          // Module
    );

    llvm::Function *func = module.getFunction(name);

    std::vector<llvm::Value *> argValues;
    for (auto &arg : args)
    {
        argValues.push_back(arg->codegen(builder, context, module));
    }
    return builder.CreateCall(func, argValues);
}

StringLiteral::StringLiteral(const std::string &value) : value(value) {}

llvm::Value *StringLiteral::codegen(llvm::IRBuilder<> &builder, llvm::LLVMContext &context, llvm::Module &module)
{
    llvm::Value *str = builder.CreateGlobalStringPtr(value);
    return str;
}