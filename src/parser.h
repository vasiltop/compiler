#ifndef PARSER_H
#define PARSER_H

#include <memory>

#include "lexer.h"
#include "compiler.h"
#include "ast.h"

class Compiler;

int getPrecedence(TokenType token);

class Parser
{
private:
    Lexer lexer;
    Compiler *compiler;

    std::string file_name;

    std::unique_ptr<FunctionCall> parseFunctionCall();
    std::unique_ptr<FunctionDecl> parseFunctionDecl();
    std::unique_ptr<ASTNode> parseExpression(int precedence);
    std::unique_ptr<ASTNode> parsePrimary();
    std::unique_ptr<ASTNode> parseUnary();
    std::unique_ptr<Include> parseInclude();

public:
    Parser(Compiler *compiler, std::string filename);
    std::vector<std::unique_ptr<ASTNode>> parse();
};

#endif