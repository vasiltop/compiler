#include "ast.h"

#include <iostream>

#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/IRBuilder.h"

static void displayStringAtIndent(int indent, const std::string &str)
{
    std::string indentStr(indent * 2, ' ');
    std::cout << indentStr << str << std::endl;
}

FunctionDecl::FunctionDecl(const std::string &name, std::vector<std::unique_ptr<ASTNode>> &args, Token &returnType) : name(name), args(std::move(args)), returnType(returnType) {}

void FunctionDecl::display(int level)
{
    displayStringAtIndent(level, "FunctionDecl: " + name);

    for (auto &arg : args)
    {
        arg->display(level + 1);
    }

    displayStringAtIndent(level, "Return Type: " + returnType.value);

    for (auto &node : body)
    {
        node->display(level + 1);
    }
}

FunctionCall::FunctionCall(const std::string &name, std::vector<std::unique_ptr<ASTNode>> &args) : name(name), args(std::move(args)) {}

void FunctionCall::display(int level)
{
    displayStringAtIndent(level, "FunctionCall: " + name);

    for (auto &arg : args)
    {
        arg->display(level + 1);
    }
}

StringLiteral::StringLiteral(const std::string &value) : value(value) {}

void StringLiteral::display(int level)
{

    displayStringAtIndent(level, "StringLiteral: " + value);
}

IntLiteral::IntLiteral(int value) : value(value) {}

void IntLiteral::display(int level)
{
    displayStringAtIndent(level, "IntLiteral: " + std::to_string(value));
}

BoolLiteral::BoolLiteral(bool value) : value(value) {}

void BoolLiteral::display(int level)
{
    displayStringAtIndent(level, "BoolLiteral: " + std::string(value ? "true" : "false"));
}

BinaryExpr::BinaryExpr(Token op, std::unique_ptr<ASTNode> lhs, std::unique_ptr<ASTNode> rhs) : op(op), lhs(std::move(lhs)), rhs(std::move(rhs)) {}

void BinaryExpr::display(int level)
{
    displayStringAtIndent(level, "BinaryExpr: " + op.value);
    lhs->display(level + 1);
    rhs->display(level + 1);
}

UnaryExpr::UnaryExpr(Token op, std::unique_ptr<ASTNode> expr) : op(op), expr(std::move(expr)) {}

void UnaryExpr::display(int level)
{
    displayStringAtIndent(level, "UnaryExpr: " + op.value);
    expr->display(level + 1);
}

Variable::Variable(const std::string &name) : name(name) {}

void Variable::display(int level)
{
    displayStringAtIndent(level, "Variable: " + name);
}

Include::Include(const std::string &filename) : filename(filename) {}

void Include::display(int level)
{
    displayStringAtIndent(level, "Include: " + filename);
}

TypedIdent::TypedIdent(const Token &type, const std::string &name) : name(name), type(type) {}

void TypedIdent::display(int level)
{
    displayStringAtIndent(level, "TypedIdent: " + name + " Type: " + type.value);
}

Return::Return(std::unique_ptr<ASTNode> expr) : expr(std::move(expr)) {}

void Return::display(int level)
{
    displayStringAtIndent(level, "Return:");
    expr->display(level + 1);
}

VariableDeclaration::VariableDeclaration(std::unique_ptr<TypedIdent> ident, std::unique_ptr<ASTNode> expr) : ident(std::move(ident)), expr(std::move(expr)) {}

void VariableDeclaration::display(int level)
{
    displayStringAtIndent(level, "VariableDeclaration:");
    ident->display(level + 1);
    expr->display(level + 1);
}