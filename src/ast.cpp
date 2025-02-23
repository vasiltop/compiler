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
    else if (stringType == "u8")
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

        if (typedIdent->type.pointerLevel > 0)
        {
            type = llvm::PointerType::get(type, 0);
        }

        argTypes.push_back(type);
    }

    llvm::Type *returnType = getLLVMType(module.getContext(), this->returnType);
    if (this->returnType.pointerLevel > 0)
    {
        returnType = llvm::PointerType::get(returnType, 0);
    }

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

            SymbolType type;
            type.type = getLLVMType(module.getContext(), args[i - 1]->type);
            type.pointerLevel = args[i - 1]->type.pointerLevel;

            parser.addVariable(arg.getName().str(), alloc, type);
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

Variable::Variable(const std::string &name, int derefCount) : name(name), derefCount(derefCount) {}

llvm::Value *Variable::codegen(llvm::IRBuilder<> &builder, llvm::Module &module, Parser &parser)
{
    auto var = parser.getVariable(name);
    llvm::Value *value = var.first;
    if (!value)
    {
        std::cerr << "Error: Unknown variable '" << name << "'." << std::endl;
        return nullptr;
    }

    llvm::Type *type = var.second.type;

    if (derefCount != var.second.pointerLevel)
    {
        type = llvm::PointerType::get(var.second.type, 0);
        for (int i = 0; i <= derefCount; i++)
        {
            value = builder.CreateLoad(type, value);
        }

        return value;
    }
    else
    {
        return builder.CreateLoad(type, value);
    }
}

void Variable::display(int level)
{
    displayStringAtIndent(level, "Variable: " + name);
    displayStringAtIndent(level + 1, "Deref Count: " + std::to_string(derefCount));
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

    SymbolType type;
    type.type = getLLVMType(module.getContext(), ident->type);
    type.pointerLevel = ident->type.pointerLevel;

    parser.addVariable(ident->name, alloc, type);

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

Reassign::Reassign(std::unique_ptr<Variable> var, std::unique_ptr<ASTNode> expr, int derefCount) : var(std::move(var)), expr(std::move(expr)), derefCount(derefCount) {}
llvm::Value *Reassign::codegen(llvm::IRBuilder<> &builder, llvm::Module &module, Parser &parser)
{
    llvm::Value *val = expr->codegen(builder, module, parser);

    if (!val)
    {
        return nullptr;
    }

    auto var = parser.getVariable(this->var->name);

    if (!var.first)
    {
        std::cerr << "Error: Unknown variable '" << this->var->name << "'." << std::endl;
        return nullptr;
    }

    llvm::Value *value = var.first;
    llvm::Type *type = var.second.type;

    if (derefCount != var.second.pointerLevel)
    {
        type = llvm::PointerType::get(var.second.type, 0);
    }

    return builder.CreateStore(val, value);
}

void Reassign::display(int level)
{
    displayStringAtIndent(level, "Reassign:");
    displayStringAtIndent(level + 1, "Deref Count: " + std::to_string(derefCount));
    var->display(level + 1);
    expr->display(level + 1);
}

Condition::Condition(
    std::unique_ptr<ASTNode> condition,
    std::optional<std::vector<std::unique_ptr<ASTNode>>> body,
    std::optional<std::vector<std::unique_ptr<ASTNode>>> elseBody)
    : condition(std::move(condition)), body(std::move(body)), elseBody(std::move(elseBody)) {}

llvm::Value *Condition::codegen(llvm::IRBuilder<> &builder, llvm::Module &module, Parser &parser)
{
    // Generate the condition value
    llvm::Value *cond = condition->codegen(builder, module, parser);

    if (!cond)
    {
        return nullptr;
    }

    llvm::Function *func = builder.GetInsertBlock()->getParent();

    llvm::BasicBlock *thenBlock = llvm::BasicBlock::Create(module.getContext(), "then", func);
    llvm::BasicBlock *elseBlock = llvm::BasicBlock::Create(module.getContext(), "else", func);
    llvm::BasicBlock *mergeBlock = llvm::BasicBlock::Create(module.getContext(), "ifcont", func);

    builder.CreateCondBr(cond, thenBlock, elseBlock);

    builder.SetInsertPoint(thenBlock);
    for (auto &node : body.value())
    {
        node->codegen(builder, module, parser);
    }
    builder.CreateBr(mergeBlock);

    builder.SetInsertPoint(elseBlock);
    if (elseBody)
    {
        for (auto &node : elseBody.value())
        {
            node->codegen(builder, module, parser);
        }
    }
    builder.CreateBr(mergeBlock);

    builder.SetInsertPoint(mergeBlock);

    return nullptr;
}

void Condition::display(int level)
{
    displayStringAtIndent(level, "Condition:");
    condition->display(level + 1);

    if (body)
    {
        displayStringAtIndent(level + 1, "Body:");
        for (auto &node : body.value())
        {
            node->display(level + 2);
        }
    }

    if (elseBody)
    {
        displayStringAtIndent(level + 1, "Else Body:");
        for (auto &node : elseBody.value())
        {
            node->display(level + 2);
        }
    }
}

While::While(std::unique_ptr<ASTNode> condition, std::optional<std::vector<std::unique_ptr<ASTNode>>> body) : condition(std::move(condition)), body(std::move(body)) {}

llvm::Value *While::codegen(llvm::IRBuilder<> &builder, llvm::Module &module, Parser &parser)
{
    llvm::Function *func = builder.GetInsertBlock()->getParent();

    llvm::BasicBlock *condBlock = llvm::BasicBlock::Create(module.getContext(), "cond", func);
    llvm::BasicBlock *bodyBlock = llvm::BasicBlock::Create(module.getContext(), "body", func);
    llvm::BasicBlock *mergeBlock = llvm::BasicBlock::Create(module.getContext(), "merge", func);

    builder.CreateBr(condBlock);

    builder.SetInsertPoint(condBlock);
    llvm::Value *cond = condition->codegen(builder, module, parser);
    builder.CreateCondBr(cond, bodyBlock, mergeBlock);

    builder.SetInsertPoint(bodyBlock);
    for (auto &node : body.value())
    {
        node->codegen(builder, module, parser);
    }
    builder.CreateBr(condBlock);

    builder.SetInsertPoint(mergeBlock);

    return nullptr;
}

void While::display(int level)
{
    displayStringAtIndent(level, "While:");
    condition->display(level + 1);

    if (body)
    {
        displayStringAtIndent(level + 1, "Body:");
        for (auto &node : body.value())
        {
            node->display(level + 2);
        }
    }
}

IndexReassign::IndexReassign(std::unique_ptr<Variable> var, std::unique_ptr<ASTNode> index, std::unique_ptr<ASTNode> expr) : var(std::move(var)), index(std::move(index)), expr(std::move(expr)) {}

llvm::Value *IndexReassign::codegen(llvm::IRBuilder<> &builder, llvm::Module &module, Parser &parser)
{
    auto v = parser.getVariable(var->name);
    if (!v.first)
    {
        std::cerr << "Error: Unknown variable '" << var->name << "'." << std::endl;
        return nullptr;
    }

    llvm::Value *i = index->codegen(builder, module, parser);
    if (!i)
    {
        return nullptr;
    }

    llvm::Value *exprValue = expr->codegen(builder, module, parser);
    if (!exprValue)
    {
        return nullptr;
    }

    llvm::Value *ptr = builder.CreateLoad(llvm::PointerType::get(v.second.type, 0), v.first);
    llvm::Value *elemPtr = builder.CreateGEP(v.second.type, ptr, {i}, "elem_ptr");

    builder.CreateStore(exprValue, elemPtr);

    return exprValue;
}

void IndexReassign::display(int level)
{
    displayStringAtIndent(level, "IndexReassign:");
    var->display(level + 1);
    index->display(level + 1);
    expr->display(level + 1);
}

VariableIndex::VariableIndex(std::unique_ptr<Variable> var, std::unique_ptr<ASTNode> index) : var(std::move(var)), index(std::move(index)) {}

llvm::Value *VariableIndex::codegen(llvm::IRBuilder<> &builder, llvm::Module &module, Parser &parser)
{
    auto v = parser.getVariable(var->name);

    if (!v.first)
    {
        std::cerr << "Error: Unknown variable '" << var->name << "'." << std::endl;
        return nullptr;
    }

    llvm::Value *i = index->codegen(builder, module, parser);

    if (!i)
    {
        return nullptr;
    }

    llvm::Value *ptr = builder.CreateLoad(llvm::PointerType::get(v.second.type, 0), v.first);
    llvm::Value *elemPtr = builder.CreateGEP(v.second.type, ptr, {i}, "elem_ptr");
    return builder.CreateLoad(v.second.type, elemPtr);
}

void VariableIndex::display(int level)
{
    displayStringAtIndent(level, "VariableIndex:");
    var->display(level + 1);
    index->display(level + 1);
}