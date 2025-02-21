#include <iostream>
#include "parser.h"

#define EXPECT_TOKEN(tokenType, errorMessage)                                      \
    if (currentToken.type != tokenType)                                            \
    {                                                                              \
        std::cerr << "filename:"                                                   \
                  << currentToken.position.row << ":" << currentToken.position.col \
                  << ": error: " << errorMessage                                   \
                  << std::endl;                                                    \
        return nullptr;                                                            \
    }

int getPrecedence(TokenType type)
{
    switch (type)
    {
    case TOKEN_OPERATOR_MUL:
    case TOKEN_OPERATOR_DIV:
    case TOKEN_OPERATOR_MOD:
        return 5;
    case TOKEN_OPERATOR_PLUS:
    case TOKEN_OPERATOR_MINUS:
        return 4;
    case TOKEN_OPERATOR_LESS:
    case TOKEN_OPERATOR_GREATER:
    case TOKEN_OPERATOR_LESS_EQUAL:
    case TOKEN_OPERATOR_GREATER_EQUAL:
        return 3;
    case TOKEN_OPERATOR_NOT_EQUAL:
    case TOKEN_OPERATOR_EQUAL:
        return 2;
    case TOKEN_OPERATOR_AND:
        return 1;
    case TOKEN_OPERATOR_OR:
        return 0;
    default:
        return -1;
    }
}

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

    EXPECT_TOKEN(TOKEN_KEYWORD_FN, "Expected keyword fn");
    currentToken = lexer.next();

    EXPECT_TOKEN(TOKEN_IDENTIFIER, "Expected identifier");
    std::string funcName = currentToken.value;
    currentToken = lexer.next();

    EXPECT_TOKEN(TOKEN_LEFT_PAREN, "Expected '('");
    currentToken = lexer.next();

    EXPECT_TOKEN(TOKEN_RIGHT_PAREN, "Expected ')'");
    currentToken = lexer.next();

    EXPECT_TOKEN(TOKEN_LEFT_BRACE, "Expected '{'");
    currentToken = lexer.next();

    auto funcDecl = std::make_unique<FunctionDecl>(funcName);

    funcDecl->body.push_back(parseFunctionCall());

    return funcDecl;
}

std::unique_ptr<FunctionCall>
Parser::parseFunctionCall()
{
    EXPECT_TOKEN(TOKEN_IDENTIFIER, "Expected identifier");
    std::string name = currentToken.value;
    currentToken = lexer.next();

    EXPECT_TOKEN(TOKEN_LEFT_PAREN, "Expected '('");
    currentToken = lexer.next();

    std::vector<std::unique_ptr<ASTNode>> args;
    while (currentToken.type != TOKEN_RIGHT_PAREN)
    {
        if (currentToken.type == TOKEN_COMMA)
        {
            currentToken = lexer.next();
            continue;
        }

        args.push_back(parseExpression(0));
    }

    EXPECT_TOKEN(TOKEN_RIGHT_PAREN, "Expected ')'");
    currentToken = lexer.next();

    EXPECT_TOKEN(TOKEN_SEMICOLON, "Expected ';'");
    currentToken = lexer.next();

    auto funcCall = std::make_unique<FunctionCall>(name, args);
    return funcCall;
}

std::unique_ptr<ASTNode>
Parser::parseExpression(int precedence = 0)
{
    auto left = parseUnary();

    while (true)
    {
        int currentPrecedence = getPrecedence(currentToken.type);

        if (currentPrecedence < precedence)
        {
            break;
        }

        Token op = currentToken;
        currentToken = lexer.next();

        auto right = parseExpression(currentPrecedence + 1);

        left = std::make_unique<BinaryExpr>(op, std::move(left), std::move(right));
    }

    return left;
}

std::unique_ptr<ASTNode>
Parser::parsePrimary()
{
    switch (currentToken.type)
    {
    case TOKEN_INT_LITERAL:
    {
        auto intLiteral = std::make_unique<IntLiteral>(std::stoi(currentToken.value));
        currentToken = lexer.next();
        return intLiteral;
    }
    case TOKEN_STRING_LITERAL:
    {
        auto stringLiteral = std::make_unique<StringLiteral>(currentToken.value);
        currentToken = lexer.next();
        return stringLiteral;
    }
    case TOKEN_BOOL_LITERAL:
    {
        auto boolLiteral = std::make_unique<BoolLiteral>(currentToken.value == "true");
        currentToken = lexer.next();
        return boolLiteral;
    }
    case TOKEN_IDENTIFIER:
    {
        std::string name = currentToken.value;

        switch (lexer.peek().type)
        {
        case TOKEN_LEFT_PAREN:
            return parseFunctionCall();
        default:
            currentToken = lexer.next();
            return std::make_unique<Variable>(name);
        }
    }
    case TOKEN_LEFT_PAREN:
    {
        currentToken = lexer.next();
        auto expr = parseExpression();
        EXPECT_TOKEN(TOKEN_RIGHT_PAREN, "Expected ')'");
        currentToken = lexer.next();
        return expr;
    }
    }

    return nullptr;
}

std::unique_ptr<ASTNode>
Parser::parseUnary()
{
    if (currentToken.type == TOKEN_OPERATOR_NOT)
    {
        Token op = currentToken;
        currentToken = lexer.next();
        auto expr = parseUnary();
        return std::make_unique<UnaryExpr>(op, std::move(expr));
    }
    return parsePrimary();
}