#ifndef AST_H
#define AST_H

#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/IRBuilder.h"

#include "lexer.h"

struct ASTNode
{
    virtual ~ASTNode() = default;
    virtual void display() = 0;
};

struct FunctionDecl : public ASTNode
{
    std::string name;
    std::vector<std::unique_ptr<ASTNode>> body;

    FunctionDecl(const std::string name);
    void display() override;
};

struct FunctionCall : public ASTNode
{
    std::string name;
    std::vector<std::unique_ptr<ASTNode>> args;

    FunctionCall(const std::string &name, std::vector<std::unique_ptr<ASTNode>> &args);
    void display() override;
};

struct StringLiteral : public ASTNode
{
    std::string value;

    StringLiteral(const std::string &value);
    void display() override;
};

struct IntLiteral : public ASTNode
{
    int value;

    IntLiteral(int value);
    void display() override;
};

struct BoolLiteral : public ASTNode
{
    bool value;

    BoolLiteral(bool value);
    void display() override;
};

struct BinaryExpr : public ASTNode
{
    Token op;
    std::unique_ptr<ASTNode> lhs;
    std::unique_ptr<ASTNode> rhs;

    BinaryExpr(Token op, std::unique_ptr<ASTNode> lhs, std::unique_ptr<ASTNode> rhs);
    void display() override;
};

struct UnaryExpr : public ASTNode
{
    Token op;
    std::unique_ptr<ASTNode> expr;

    UnaryExpr(Token op, std::unique_ptr<ASTNode> expr);
    void display() override;
};

struct Variable : public ASTNode
{
    std::string name;

    Variable(const std::string &name);
    void display() override;
};

#endif