#ifndef PARSER_H
#define PARSER_H

#include <memory>

#include "lexer.h"
#include "ast.h"

int getPrecedence(TokenType token);

class Parser
{
private:
    Lexer &lexer;
    Token currentToken;

    std::string file_name;

    std::unique_ptr<FunctionCall> parseFunctionCall();
    std::unique_ptr<FunctionDecl> parseFunctionDecl();
    std::unique_ptr<ASTNode> parseExpression(int precedence);
    std::unique_ptr<ASTNode> parsePrimary();
    std::unique_ptr<ASTNode> parseUnary();

public:
    Parser(Lexer &lexer);
    std::vector<std::unique_ptr<ASTNode>> ast();
};

#endif