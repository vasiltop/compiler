#include "ast.h"

#include <iostream>

#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/IRBuilder.h"

static llvm::Type *getLLVMType(llvm::LLVMContext &context, const Type &type)
{
    std::string stringType = type.type.value;
    llvm::Type *t = nullptr;

    if (stringType == "i32")
    {
        t = llvm::Type::getInt32Ty(context);
    }
    else if (stringType == "i8")
    {
        t = llvm::Type::getInt8Ty(context);
    }
    else if (stringType == "bool")
    {
        t = llvm::Type::getInt1Ty(context);
    }
    else if (stringType == "void")
    {
        t = llvm::Type::getVoidTy(context);
    }
    else
    {
        std::cerr << "Unknown type: " << stringType << std::endl;
    }

    for (int i = 0; i < type.pointerLevel; i++)
    {
        t = llvm::PointerType::get(t, 0);
    }

    return t;
}

static void displayStringAtIndent(int indent, const std::string &str)
{
    std::string indentStr(indent * 2, ' ');
    std::cout << indentStr << str << std::endl;
}

FunctionDecl::FunctionDecl(const std::string &name, std::vector<std::unique_ptr<TypedIdent>> &args, std::unique_ptr<Type> returnType) : name(name), args(std::move(args)), returnType(returnType->type, returnType->pointerLevel) {}

llvm::Value *FunctionDecl::codegen(llvm::IRBuilder<> &builder, llvm::Module &module, Parser &parser)
{
    std::vector<llvm::Type *> argTypes;

    for (auto &arg : args)
    {
        TypedIdent *typedIdent = arg.get();
        llvm::Type *type = getLLVMType(module.getContext(), typedIdent->type);
        argTypes.push_back(type);
    }

    llvm::Type *returnType = getLLVMType(module.getContext(), this->returnType);
    llvm::FunctionType *funcType = llvm::FunctionType::get(returnType, argTypes, false);

    llvm::Function *func = llvm::Function::Create(funcType, llvm::Function::ExternalLinkage, name, &module);

    if (body)
    {

        llvm::BasicBlock *entry = llvm::BasicBlock::Create(module.getContext(), "entry", func);
        builder.SetInsertPoint(entry);

        unsigned i = 0;
        for (auto &arg : func->args())
        {
            llvm::AllocaInst *alloc = builder.CreateAlloca(arg.getType(), nullptr, arg.getName());
            builder.CreateStore(&arg, alloc);
            arg.setName(args[i++]->name);
            parser.addVariable(arg.getName().str(), alloc, arg.getType());
        }

        for (auto &node : body.value())
        {
            node->codegen(builder, module, parser);
        }
    }
    return func;
}

void FunctionDecl::display(int level)
{
    displayStringAtIndent(level, "FunctionDecl: " + name);

    for (auto &arg : args)
    {
        arg->display(level + 1);
    }

    returnType.display(level + 1);

    if (body)
    {
        for (auto &node : body.value())
        {
            node->display(level + 1);
        }
    }
}

FunctionCall::FunctionCall(const std::string &name, std::vector<std::unique_ptr<ASTNode>> &args) : name(name), args(std::move(args)) {}

llvm::Value *FunctionCall::codegen(llvm::IRBuilder<> &builder, llvm::Module &module, Parser &parser)
{
    llvm::Function *func = module.getFunction(name);

    if (!func)
    {
        std::cerr << "Unknown function: " << name << std::endl;
        return nullptr;
    }

    std::vector<llvm::Value *> callArgs;

    for (auto &arg : args)
    {
        callArgs.push_back(arg->codegen(builder, module, parser));
    }

    return builder.CreateCall(func, callArgs);
}

void FunctionCall::display(int level)
{
    displayStringAtIndent(level, "FunctionCall: " + name);

    for (auto &arg : args)
    {
        arg->display(level + 1);
    }
}

StringLiteral::StringLiteral(const std::string &value) : value(value) {}

llvm::Value *StringLiteral::codegen(llvm::IRBuilder<> &builder, llvm::Module &module, Parser &parser)
{
    return builder.CreateGlobalStringPtr(value);
}

void StringLiteral::display(int level)
{
    displayStringAtIndent(level, "StringLiteral: " + value);
}

IntLiteral::IntLiteral(int value) : value(value) {}

llvm::Value *IntLiteral::codegen(llvm::IRBuilder<> &builder, llvm::Module &module, Parser &parser)
{
    return builder.getInt32(value);
}

void IntLiteral::display(int level)
{
    displayStringAtIndent(level, "IntLiteral: " + std::to_string(value));
}

BoolLiteral::BoolLiteral(bool value) : value(value) {}

llvm::Value *BoolLiteral::codegen(llvm::IRBuilder<> &builder, llvm::Module &module, Parser &parser)
{
    return builder.getInt1(value);
}

void BoolLiteral::display(int level)
{
    displayStringAtIndent(level, "BoolLiteral: " + std::string(value ? "true" : "false"));
}

BinaryExpr::BinaryExpr(Token op, std::unique_ptr<ASTNode> lhs, std::unique_ptr<ASTNode> rhs) : op(op), lhs(std::move(lhs)), rhs(std::move(rhs)) {}

llvm::Value *BinaryExpr::codegen(llvm::IRBuilder<> &builder, llvm::Module &module, Parser &parser)
{
    llvm::Value *lhsValue = lhs->codegen(builder, module, parser);
    llvm::Value *rhsValue = rhs->codegen(builder, module, parser);

    if (!lhsValue || !rhsValue)
    {
        return nullptr;
    }

    switch (op.type)
    {
    case TOKEN_OPERATOR_PLUS:
        return builder.CreateAdd(lhsValue, rhsValue);
    case TOKEN_OPERATOR_MINUS:
        return builder.CreateSub(lhsValue, rhsValue);
    case TOKEN_OPERATOR_MUL:
        return builder.CreateMul(lhsValue, rhsValue);
    case TOKEN_OPERATOR_DIV:
        return builder.CreateSDiv(lhsValue, rhsValue);
    case TOKEN_OPERATOR_EQUAL:
        return builder.CreateICmpEQ(lhsValue, rhsValue);
    case TOKEN_OPERATOR_NOT_EQUAL:
        return builder.CreateICmpNE(lhsValue, rhsValue);
    case TOKEN_OPERATOR_LESS:
        return builder.CreateICmpSLT(lhsValue, rhsValue);
    case TOKEN_OPERATOR_GREATER:
        return builder.CreateICmpSGT(lhsValue, rhsValue);
    case TOKEN_OPERATOR_LESS_EQUAL:
        return builder.CreateICmpSLE(lhsValue, rhsValue);
    case TOKEN_OPERATOR_GREATER_EQUAL:
        return builder.CreateICmpSGE(lhsValue, rhsValue);
    case TOKEN_OPERATOR_AND:
        return builder.CreateAnd(lhsValue, rhsValue);
    case TOKEN_OPERATOR_OR:
        return builder.CreateOr(lhsValue, rhsValue);
    default:
        return nullptr;
    }
}

void BinaryExpr::display(int level)
{
    displayStringAtIndent(level, "BinaryExpr: " + op.value);
    lhs->display(level + 1);
    rhs->display(level + 1);
}

UnaryExpr::UnaryExpr(Token op, std::unique_ptr<ASTNode> expr) : op(op), expr(std::move(expr)) {}

llvm::Value *UnaryExpr::codegen(llvm::IRBuilder<> &builder, llvm::Module &module, Parser &parser)
{
    llvm::Value *val = expr->codegen(builder, module, parser);

    if (!val)
    {
        return nullptr;
    }

    if (op.value == "-")
    {
        return builder.CreateNeg(val);
    }
    else if (op.value == "!")
    {
        return builder.CreateNot(val);
    }

    return val;
}

void UnaryExpr::display(int level)
{
    displayStringAtIndent(level, "UnaryExpr: " + op.value);
    expr->display(level + 1);
}

Variable::Variable(const std::string &name) : name(name) {}

llvm::Value *Variable::codegen(llvm::IRBuilder<> &builder, llvm::Module &module, Parser &parser)
{
    auto var = parser.getVariable(name);
    llvm::Value *value = var.first;
    if (!value)
    {
        std::cerr << "Error: Unknown variable '" << name << "'." << std::endl;
        return nullptr;
    }

    return builder.CreateLoad(var.second, value, name.c_str());
}

void Variable::display(int level)
{
    displayStringAtIndent(level, "Variable: " + name);
}

Include::Include(const std::string &filename) : filename(filename) {}

llvm::Value *Include::codegen(llvm::IRBuilder<> &builder, llvm::Module &module, Parser &parser)
{
    return nullptr; // not used
}

void Include::display(int level)
{
    displayStringAtIndent(level, "Include: " + filename);
}

TypedIdent::TypedIdent(std::unique_ptr<Type> type, const std::string &name) : name(name), type(type->type, type->pointerLevel) {}

llvm::Value *TypedIdent::codegen(llvm::IRBuilder<> &builder, llvm::Module &module, Parser &parser)
{
    return nullptr; // not used
}

void TypedIdent::display(int level)
{
    displayStringAtIndent(level, "TypedIdent: " + name);
    type.display(level + 1);
}

Return::Return(std::unique_ptr<ASTNode> expr) : expr(std::move(expr)) {}

llvm::Value *Return::codegen(llvm::IRBuilder<> &builder, llvm::Module &module, Parser &parser)
{
    llvm::Value *retValue = expr->codegen(builder, module, parser);

    if (!retValue)
    {
        return nullptr;
    }

    return builder.CreateRet(retValue);
}

void Return::display(int level)
{
    displayStringAtIndent(level, "Return:");
    expr->display(level + 1);
}

VariableDeclaration::VariableDeclaration(std::unique_ptr<TypedIdent> ident, std::unique_ptr<ASTNode> expr) : ident(std::move(ident)), expr(std::move(expr)) {}

llvm::Value *VariableDeclaration::codegen(llvm::IRBuilder<> &builder, llvm::Module &module, Parser &parser)
{
    llvm::Value *val = expr->codegen(builder, module, parser);

    if (!val)
    {
        return nullptr;
    }

    llvm::AllocaInst *alloc = builder.CreateAlloca(val->getType(), nullptr, ident->name);
    builder.CreateStore(val, alloc);
    parser.addVariable(ident->name, alloc, val->getType());

    return alloc;
}

void VariableDeclaration::display(int level)
{
    displayStringAtIndent(level, "VariableDeclaration:");
    ident->display(level + 1);
    expr->display(level + 1);
}

Type::Type(Token type, int pointerLevel) : type(type), pointerLevel(pointerLevel) {}

llvm::Value *Type::codegen(llvm::IRBuilder<> &builder, llvm::Module &module, Parser &parser)
{
    return nullptr; // not used
}

void Type::display(int level)
{
    displayStringAtIndent(level, "Type: " + type.value + " Pointer Level: " + std::to_string(pointerLevel));
}