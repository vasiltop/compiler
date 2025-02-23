#ifndef PARSER_H
#define PARSER_H

#include <memory>

#include "lexer.h"
#include "compiler.h"
#include "ast.h"

class Compiler;
class ASTNode;
class FunctionCall;
class FunctionDecl;
class VariableDeclaration;
class TypedIdent;
class Return;
class Include;
class Type;
class Reassign;
class Condition;
class While;
class IndexReassign;
class VariableIndex;
class StructDeclaration;
class StructReassign;
class StructField;

int getPrecedence(TokenType token);

struct SymbolType
{
    llvm::Type *type;
    int pointerLevel;
};

class Parser
{
private:
    Lexer lexer;
    Compiler *compiler;

    std::string file_name;

    std::unique_ptr<FunctionCall> parseFunctionCall(std::string name, bool consumeSemicolon);
    std::unique_ptr<FunctionDecl> parseFunctionDecl();
    std::unique_ptr<VariableDeclaration> parseVariableDeclaration();
    std::unique_ptr<StructDeclaration> parseStructDeclaration();
    std::unique_ptr<TypedIdent> parseTypedIdent();
    std::unique_ptr<Return> parseReturn();
    std::unique_ptr<ASTNode> parseExpression(int precedence);
    std::unique_ptr<ASTNode> parsePrimary();
    std::unique_ptr<ASTNode> parseUnary();
    std::unique_ptr<Include> parseInclude();
    std::unique_ptr<ASTNode> parseNext();
    std::unique_ptr<IndexReassign> parseIndexReassign(std::string name);
    std::unique_ptr<Type> parseType();
    std::unique_ptr<Reassign> parseReassign(std::string name);
    std::unique_ptr<Condition> parseCondition();
    std::unique_ptr<While> parseWhile();
    std::unique_ptr<StructReassign> parseStructReassign(std::string name);
    std::unordered_map<std::string, std::pair<llvm::Value *, SymbolType>> symbolTable;
    std::unordered_map<std::string, std::pair<llvm::StructType *, std::vector<std::string>>> structTable;

public:
    Parser(Compiler *compiler, std::string filename);
    std::vector<std::unique_ptr<ASTNode>> parse();

    void addVariable(const std::string &name, llvm::Value *value, SymbolType type);
    std::pair<llvm::Value *, SymbolType> getVariable(const std::string &name);

    void addStructType(const std::string &name, llvm::StructType *type, std::vector<std::string> fields);
    std::pair<llvm::StructType *, std::vector<std::string>> getStructType(const std::string &name);
};

#endif