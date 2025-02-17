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

public:
    Parser(Lexer &lexer);

    std::unique_ptr<FunctionDecl> parseFunctionDecl();
    std::unique_ptr<FunctionCall> parseFunctionCall();
};

#endif