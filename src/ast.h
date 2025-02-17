#ifndef AST_H
#define AST_H

#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/IRBuilder.h"

struct ASTNode
{
    virtual ~ASTNode() = default;
    virtual llvm::Value *codegen(llvm::IRBuilder<> &builder, llvm::LLVMContext &context, llvm::Module &module) = 0;
};

struct FunctionDecl : public ASTNode
{
    std::string name;
    std::vector<std::unique_ptr<ASTNode>> body;

    FunctionDecl(const std::string name);
    llvm::Value *codegen(llvm::IRBuilder<> &builder, llvm::LLVMContext &context, llvm::Module &module) override;
};

struct FunctionCall : public ASTNode
{
    std::string name;
    std::vector<std::unique_ptr<ASTNode>> args;

    FunctionCall(const std::string &name);
    llvm::Value *codegen(llvm::IRBuilder<> &builder, llvm::LLVMContext &context, llvm::Module &module) override;
};

struct StringLiteral : public ASTNode
{
    std::string value;

    StringLiteral(const std::string &value);
    llvm::Value *codegen(llvm::IRBuilder<> &builder, llvm::LLVMContext &context, llvm::Module &module) override;
};

#endif