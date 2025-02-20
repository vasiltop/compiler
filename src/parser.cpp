#include <iostream>
#include "parser.h"

Parser::Parser(Lexer &lexer) : lexer(lexer)
{
    currentToken = lexer.next();
}

std::vector<std::unique_ptr<ASTNode>> Parser::ast()
{
    std::vector<std::unique_ptr<ASTNode>> nodes;
    while (currentToken.type != TOKEN_EOF)
    {
        switch (currentToken.type)
        {
        case TOKEN_KEYWORD_FN:
            nodes.push_back(parseFunctionDecl());
            break;
        case TOKEN_IDENTIFIER:
            switch (lexer.peek().type)
            {
            case TOKEN_LEFT_PAREN:
                nodes.push_back(parseFunctionCall());
                break;
            }
            break;
        }

        currentToken = lexer.next();
    }
    return nodes;
}

std::unique_ptr<FunctionDecl> Parser::parseFunctionDecl()
{

    // Expect "fn"
    if (currentToken.type != TOKEN_KEYWORD_FN)
    {
        std::cerr << "Expected 'fn'" << std::endl;
        return nullptr;
    }
    currentToken = lexer.next();

    // Expect function name
    if (currentToken.type != TOKEN_IDENTIFIER)
    {
        std::cerr << "Expected function name" << std::endl;
        return nullptr;
    }
    std::string funcName = currentToken.value;
    currentToken = lexer.next();

    // Expect "(" and ")"
    if (currentToken.type != TOKEN_LEFT_PAREN)
    {
        std::cerr << "Expected '('" << std::endl;
        return nullptr;
    }
    currentToken = lexer.next();

    if (currentToken.type != TOKEN_RIGHT_PAREN)
    {
        std::cerr << "Expected ')'" << std::endl;
        return nullptr;
    }
    currentToken = lexer.next();

    // Expect "{"
    if (currentToken.type != TOKEN_LEFT_BRACE)
    {
        std::cerr << "Expected '{'" << std::endl;
        return nullptr;
    }

    currentToken = lexer.next();
    auto funcDecl = std::make_unique<FunctionDecl>(funcName);

    funcDecl->body.push_back(parseFunctionCall());

    return funcDecl;
}

std::unique_ptr<FunctionCall>
Parser::parseFunctionCall()
{
    // Expect "printf"
    if (currentToken.type != TOKEN_IDENTIFIER)
    {
        std::cerr << "Expected 'identifier' in function call, received: " << currentToken.type << std::endl;
    }
    std::string name = currentToken.value;
    currentToken = lexer.next();

    // Expect "("
    if (currentToken.type != TOKEN_LEFT_PAREN)
    {
        std::cerr << "Expected '('" << std::endl;
        return nullptr;
    }
    currentToken = lexer.next();

    // Expect string literal
    if (currentToken.type != TOKEN_STRING_LITERAL)
    {
        std::cerr << "Expected string literal" << std::endl;
        return nullptr;
    }
    std::string strValue = currentToken.value;
    currentToken = lexer.next();

    // Expect ")"
    if (currentToken.type != TOKEN_RIGHT_PAREN)
    {
        std::cerr << "Expected ')'" << std::endl;
        return nullptr;
    }
    currentToken = lexer.next();

    // Expect ";"
    if (currentToken.type != TOKEN_SEMICOLON)
    {
        std::cerr << "Expected ';'" << std::endl;
        return nullptr;
    }
    currentToken = lexer.next();

    auto funcCall = std::make_unique<FunctionCall>(name);
    funcCall->args.push_back(std::make_unique<StringLiteral>(strValue));
    return funcCall;
}