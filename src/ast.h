#ifndef AST_H
#define AST_H

#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/IRBuilder.h"

#include "parser.h"
#include "lexer.h"

class Parser;

static void displayStringAtIndent(int indent, const std::string &str);

struct ASTNode
{
    virtual ~ASTNode() = default;
    virtual llvm::Value *codegen(llvm::IRBuilder<> &builder, llvm::Module &module, Parser &parser) = 0;
    virtual void display(int level) = 0;
};

struct Type : public ASTNode
{
    Token type;
    int pointerLevel;

    Type(Token type, int pointerLevel);
    llvm::Value *codegen(llvm::IRBuilder<> &builder, llvm::Module &module, Parser &parser) override;
    void display(int level) override;
};

struct FunctionCall : public ASTNode
{
    std::string name;
    std::vector<std::unique_ptr<ASTNode>> args;

    FunctionCall(const std::string &name, std::vector<std::unique_ptr<ASTNode>> &args);
    llvm::Value *codegen(llvm::IRBuilder<> &builder, llvm::Module &module, Parser &parser) override;
    void display(int level) override;
};

struct StringLiteral : public ASTNode
{
    std::string value;

    StringLiteral(const std::string &value);
    llvm::Value *codegen(llvm::IRBuilder<> &builder, llvm::Module &module, Parser &parser) override;
    void display(int level) override;
};

struct IntLiteral : public ASTNode
{
    int value;

    IntLiteral(int value);
    llvm::Value *codegen(llvm::IRBuilder<> &builder, llvm::Module &module, Parser &parser) override;
    void display(int level) override;
};

struct BoolLiteral : public ASTNode
{
    bool value;

    BoolLiteral(bool value);
    llvm::Value *codegen(llvm::IRBuilder<> &builder, llvm::Module &module, Parser &parser) override;
    void display(int level) override;
};

struct BinaryExpr : public ASTNode
{
    Token op;
    std::unique_ptr<ASTNode> lhs;
    std::unique_ptr<ASTNode> rhs;

    BinaryExpr(Token op, std::unique_ptr<ASTNode> lhs, std::unique_ptr<ASTNode> rhs);
    llvm::Value *codegen(llvm::IRBuilder<> &builder, llvm::Module &module, Parser &parser) override;
    void display(int level) override;
};

struct UnaryExpr : public ASTNode
{
    Token op;
    std::unique_ptr<ASTNode> expr;

    UnaryExpr(Token op, std::unique_ptr<ASTNode> expr);
    llvm::Value *codegen(llvm::IRBuilder<> &builder, llvm::Module &module, Parser &parser) override;
    void display(int level) override;
};

struct Variable : public ASTNode
{
    std::string name;
    int derefCount;

    Variable(const std::string &name, int derefCount);
    llvm::Value *codegen(llvm::IRBuilder<> &builder, llvm::Module &module, Parser &parser) override;
    void display(int level) override;
};

struct Include : public ASTNode
{
    std::string filename;

    Include(const std::string &filename);
    llvm::Value *codegen(llvm::IRBuilder<> &builder, llvm::Module &module, Parser &parser) override;
    void display(int level) override;
};

struct TypedIdent : public ASTNode
{
    Type type;
    std::string name;

    TypedIdent(std::unique_ptr<Type> type, const std::string &name);
    llvm::Value *codegen(llvm::IRBuilder<> &builder, llvm::Module &module, Parser &parser) override;
    void display(int level) override;
};

struct FunctionDecl : public ASTNode
{
    std::string name;
    std::vector<std::unique_ptr<TypedIdent>> args;
    Type returnType;
    std::optional<std::vector<std::unique_ptr<ASTNode>>> body;

    FunctionDecl(const std::string &name, std::vector<std::unique_ptr<TypedIdent>> &args, std::unique_ptr<Type> returnType);
    llvm::Value *codegen(llvm::IRBuilder<> &builder, llvm::Module &module, Parser &parser) override;
    void display(int level) override;
};

struct Return : public ASTNode
{
    std::unique_ptr<ASTNode> expr;

    Return(std::unique_ptr<ASTNode> expr);
    llvm::Value *codegen(llvm::IRBuilder<> &builder, llvm::Module &module, Parser &parser) override;
    void display(int level) override;
};

struct VariableDeclaration : public ASTNode
{
    std::unique_ptr<TypedIdent> ident;
    std::unique_ptr<ASTNode> expr;

    VariableDeclaration(std::unique_ptr<TypedIdent> ident, std::unique_ptr<ASTNode> expr);
    llvm::Value *codegen(llvm::IRBuilder<> &builder, llvm::Module &module, Parser &parser) override;
    void display(int level) override;
};

struct Reassign : public ASTNode
{
    std::unique_ptr<Variable> var;
    std::unique_ptr<ASTNode> expr;
    int derefCount;

    Reassign(std::unique_ptr<Variable> var, std::unique_ptr<ASTNode> expr, int derefCount);
    llvm::Value *codegen(llvm::IRBuilder<> &builder, llvm::Module &module, Parser &parser) override;
    void display(int level) override;
};

struct Condition : public ASTNode
{
    std::unique_ptr<ASTNode> condition;
    std::optional<std::vector<std::unique_ptr<ASTNode>>> body;
    std::optional<std::vector<std::unique_ptr<ASTNode>>> elseBody;

    Condition(std::unique_ptr<ASTNode> condition, std::optional<std::vector<std::unique_ptr<ASTNode>>> body, std::optional<std::vector<std::unique_ptr<ASTNode>>> elseBody);
    llvm::Value *codegen(llvm::IRBuilder<> &builder, llvm::Module &module, Parser &parser) override;
    void display(int level) override;
};

struct While : public ASTNode
{
    std::unique_ptr<ASTNode> condition;
    std::optional<std::vector<std::unique_ptr<ASTNode>>> body;

    While(std::unique_ptr<ASTNode> condition, std::optional<std::vector<std::unique_ptr<ASTNode>>> body);
    llvm::Value *codegen(llvm::IRBuilder<> &builder, llvm::Module &module, Parser &parser) override;
    void display(int level) override;
};

struct IndexReassign : public ASTNode
{
    std::unique_ptr<Variable> var;
    std::unique_ptr<ASTNode> index;
    std::unique_ptr<ASTNode> expr;

    IndexReassign(std::unique_ptr<Variable> var, std::unique_ptr<ASTNode> index, std::unique_ptr<ASTNode> expr);
    llvm::Value *codegen(llvm::IRBuilder<> &builder, llvm::Module &module, Parser &parser) override;
    void display(int level) override;
};

struct VariableIndex : public ASTNode
{
    std::unique_ptr<Variable> var;
    std::unique_ptr<ASTNode> index;

    VariableIndex(std::unique_ptr<Variable> var, std::unique_ptr<ASTNode> index);
    llvm::Value *codegen(llvm::IRBuilder<> &builder, llvm::Module &module, Parser &parser) override;
    void display(int level) override;
};

struct StructField : public ASTNode
{
    std::unique_ptr<Variable> var;
    std::string member;

    StructField(std::unique_ptr<Variable> var, std::string &member);
    llvm::Value *codegen(llvm::IRBuilder<> &builder, llvm::Module &module, Parser &parser) override;
    void display(int level) override;
};

struct StructDeclaration : public ASTNode
{
    std::string name;
    std::vector<std::unique_ptr<TypedIdent>> members;

    StructDeclaration(const std::string &name, std::vector<std::unique_ptr<TypedIdent>> &members);
    llvm::Value *codegen(llvm::IRBuilder<> &builder, llvm::Module &module, Parser &parser) override;
    void display(int level) override;
};

struct StructReassign : public ASTNode
{
    std::unique_ptr<Variable> var;
    std::string member;
    std::unique_ptr<ASTNode> expr;

    StructReassign(std::unique_ptr<Variable> var, std::string &member, std::unique_ptr<ASTNode> expr);
    llvm::Value *codegen(llvm::IRBuilder<> &builder, llvm::Module &module, Parser &parser) override;
    void display(int level) override;
};

#endif