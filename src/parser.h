#ifndef PARSER_H
#define PARSER_H

#include <memory>

#include "lexer.h"
#include "ast.h"

class Parser
{
private:
    Lexer &lexer;
    Token currentToken;

    std::unique_ptr<FunctionCall> parseFunctionCall();
    std::unique_ptr<FunctionDecl> parseFunctionDecl();

public:
    Parser(Lexer &lexer);
    std::vector<std::unique_ptr<ASTNode>> ast();
};

#endif