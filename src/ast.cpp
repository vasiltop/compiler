#include "ast.h"
#include <iostream>

#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/IRBuilder.h"

FunctionDecl::FunctionDecl(const std::string name) : name(name) {}

void FunctionDecl::display()
{
    std::cout << "FunctionDecl: " << name << std::endl;

    for (auto &node : body)
    {
        node->display();
    }
}

FunctionCall::FunctionCall(const std::string &name, std::vector<std::unique_ptr<ASTNode>> &args) : name(name), args(std::move(args)) {}

void FunctionCall::display()
{
    std::cout << "FunctionCall: " << name << std::endl;

    for (auto &arg : args)
    {
        arg->display();
    }
}

StringLiteral::StringLiteral(const std::string &value) : value(value) {}

void StringLiteral::display()
{
    std::cout << "StringLiteral: " << value << std::endl;
}

IntLiteral::IntLiteral(int value) : value(value) {}

void IntLiteral::display()
{
    std::cout << "IntLiteral: " << value << std::endl;
}

BoolLiteral::BoolLiteral(bool value) : value(value) {}

void BoolLiteral::display()
{
    std::cout << "BoolLiteral: " << value << std::endl;
}

BinaryExpr::BinaryExpr(Token op, std::unique_ptr<ASTNode> lhs, std::unique_ptr<ASTNode> rhs) : op(op), lhs(std::move(lhs)), rhs(std::move(rhs)) {}

void BinaryExpr::display()
{
    std::cout << "BinaryExpr: " << op.value << std::endl;
    std::cout << "LHS: " << std::endl;
    lhs->display();
    std::cout << "RHS: " << std::endl;
    rhs->display();
}

UnaryExpr::UnaryExpr(Token op, std::unique_ptr<ASTNode> expr) : op(op), expr(std::move(expr)) {}

void UnaryExpr::display()
{
    std::cout << "UnaryExpr: " << op.value << std::endl;
    expr->display();
}

Variable::Variable(const std::string &name) : name(name) {}

void Variable::display()
{
    std::cout << "Variable: " << name << std::endl;
}
