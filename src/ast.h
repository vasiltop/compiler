#ifndef AST_H
#define AST_H

#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/IRBuilder.h"

#include "lexer.h"

static void displayStringAtIndent(int indent, const std::string &str);

struct ASTNode
{
    virtual ~ASTNode() = default;
    virtual void display(int level) = 0;
};

struct FunctionDecl : public ASTNode
{
    std::string name;
    std::vector<std::unique_ptr<ASTNode>> args;
    Token returnType;
    std::vector<std::unique_ptr<ASTNode>> body;

    FunctionDecl(const std::string &name, std::vector<std::unique_ptr<ASTNode>> &args, Token &returnType);
    void display(int level) override;
};

struct FunctionCall : public ASTNode
{
    std::string name;
    std::vector<std::unique_ptr<ASTNode>> args;

    FunctionCall(const std::string &name, std::vector<std::unique_ptr<ASTNode>> &args);
    void display(int level) override;
};

struct StringLiteral : public ASTNode
{
    std::string value;

    StringLiteral(const std::string &value);
    void display(int level) override;
};

struct IntLiteral : public ASTNode
{
    int value;

    IntLiteral(int value);
    void display(int level) override;
};

struct BoolLiteral : public ASTNode
{
    bool value;

    BoolLiteral(bool value);
    void display(int level) override;
};

struct BinaryExpr : public ASTNode
{
    Token op;
    std::unique_ptr<ASTNode> lhs;
    std::unique_ptr<ASTNode> rhs;

    BinaryExpr(Token op, std::unique_ptr<ASTNode> lhs, std::unique_ptr<ASTNode> rhs);
    void display(int level) override;
    void displayInner(int level);
};

struct UnaryExpr : public ASTNode
{
    Token op;
    std::unique_ptr<ASTNode> expr;

    UnaryExpr(Token op, std::unique_ptr<ASTNode> expr);
    void display(int level) override;
};

struct Variable : public ASTNode
{
    std::string name;

    Variable(const std::string &name);
    void display(int level) override;
};

struct Include : public ASTNode
{
    std::string filename;

    Include(const std::string &filename);
    void display(int level) override;
};

struct TypedIdent : public ASTNode
{
    Token type;
    std::string name;

    TypedIdent(const Token &type, const std::string &name);
    void display(int level) override;
};

struct Return : public ASTNode
{
    std::unique_ptr<ASTNode> expr;

    Return(std::unique_ptr<ASTNode> expr);
    void display(int level) override;
};

#endif