#include <iostream>
#include "parser.h"
#include "compiler.h"

#define PANIC(message)                                                                                                \
    do                                                                                                                \
    {                                                                                                                 \
        std::cerr << "PANIC at " << __FILE__ << ":" << __LINE__ << " (" << __func__ << "): " << message << std::endl; \
        std::exit(EXIT_FAILURE);                                                                                      \
    } while (0)

#define EXPECT_TOKEN(tokenType, errorMessage)                                      \
    if (lexer.peek().type != tokenType)                                            \
    {                                                                              \
        std::cerr << lexer.input.filename << ":"                                   \
                  << lexer.peek().position.row << ":" << lexer.peek().position.col \
                  << ": error: " << errorMessage                                   \
                  << " Received: " << lexer.peek().value                           \
                  << std::endl;                                                    \
        PANIC("Unexpected token");                                                 \
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

Parser::Parser(Compiler *compiler, std::string filename) : compiler(compiler), lexer(filename)
{
    // lexer.dumpTokens();
}

void Parser::addVariable(const std::string &name, llvm::Value *value, SymbolType type)
{
    symbolTable.insert({name, {value, type}});
}

std::pair<llvm::Value *, SymbolType> Parser::getVariable(const std::string &name)
{
    if (symbolTable.find(name) == symbolTable.end())
    {
        return {nullptr, {}};
    }

    return symbolTable[name];
}

void Parser::addStructType(const std::string &name, llvm::StructType *type, std::vector<std::string> fields)
{
    structTable.insert({name, {type, fields}});
}

std::pair<llvm::StructType *, std::vector<std::string>> Parser::getStructType(const std::string &name)
{
    if (structTable.find(name) == structTable.end())
    {
        return {nullptr, {}};
    }

    return structTable[name];
}

std::vector<std::unique_ptr<ASTNode>> Parser::parse()
{
    std::vector<std::unique_ptr<ASTNode>> nodes;
    while (lexer.peek().type != TOKEN_EOF)
    {
        nodes.push_back(parseNext());
    }

    return nodes;
}

std::unique_ptr<ASTNode> Parser::parseNext()
{
    Token token = lexer.peek();
    Token next = lexer.next();

    switch (token.type)
    {
    case TOKEN_KEYWORD_FN:
        return parseFunctionDecl();

    case TOKEN_KEYWORD_STRUCT:
        return parseStructDeclaration();

    case TOKEN_KEYWORD_RETURN:
        return parseReturn();

    case TOKEN_IDENTIFIER:
        switch (lexer.peek().type)
        {
        case TOKEN_LEFT_PAREN:
            return parseFunctionCall(token.value, true);

        case TOKEN_OPERATOR_ASSIGN:
        case TOKEN_POINTER:
            return parseReassign(token.value);
        case TOKEN_LEFT_SQUARE_BRACKET:
            return parseIndexReassign(token.value);
        case TOKEN_DOT:
            return parseStructReassign(token.value);
        }

        break;

    case TOKEN_HASHTAG:
        return parseInclude();
    case TOKEN_KEYWORD_LET:
        return parseVariableDeclaration();
    case TOKEN_KEYWORD_IF:
        return parseCondition();
    case TOKEN_KEYWORD_WHILE:
        return parseWhile();
    }

    return nullptr;
}

std::unique_ptr<StructReassign> Parser::parseStructReassign(std::string name)
{
    EXPECT_TOKEN(TOKEN_DOT, "Expected '.'");
    lexer.next();

    EXPECT_TOKEN(TOKEN_IDENTIFIER, "Expected identifier");
    std::string member = lexer.peek().value;
    lexer.next();

    EXPECT_TOKEN(TOKEN_OPERATOR_ASSIGN, "Expected '='");
    lexer.next();

    auto expr = parseExpression(0);

    EXPECT_TOKEN(TOKEN_SEMICOLON, "Expected ';");
    lexer.next();

    return std::make_unique<StructReassign>(std::make_unique<Variable>(name, 0), member, std::move(expr));
}

std::unique_ptr<StructDeclaration> Parser::parseStructDeclaration()
{
    EXPECT_TOKEN(TOKEN_IDENTIFIER, "Expected identifier for struct");
    std::string name = lexer.peek().value;
    lexer.next();

    EXPECT_TOKEN(TOKEN_LEFT_BRACE, "Expected '{'");
    lexer.next();

    std::vector<std::unique_ptr<TypedIdent>> members;
    while (lexer.peek().type != TOKEN_RIGHT_BRACE)
    {
        members.push_back(parseTypedIdent());
        EXPECT_TOKEN(TOKEN_COMMA, "Expected ',");
        lexer.next();
    }

    EXPECT_TOKEN(TOKEN_RIGHT_BRACE, "Expected '}'");
    lexer.next();

    return std::make_unique<StructDeclaration>(name, members);
}

std::unique_ptr<While> Parser::parseWhile()
{
    auto condition = parseExpression(0);

    EXPECT_TOKEN(TOKEN_LEFT_BRACE, "Expected '{'");
    lexer.next();

    std::optional<std::vector<std::unique_ptr<ASTNode>>> body = std::vector<std::unique_ptr<ASTNode>>();

    while (lexer.peek().type != TOKEN_RIGHT_BRACE)
    {
        body.value().push_back(parseNext());
    }

    EXPECT_TOKEN(TOKEN_RIGHT_BRACE, "Expected '}'");
    lexer.next();

    return std::make_unique<While>(std::move(condition), std::move(body));
}

std::unique_ptr<Condition> Parser::parseCondition()
{

    auto condition = parseExpression(0);

    EXPECT_TOKEN(TOKEN_LEFT_BRACE, "Expected '{'");
    lexer.next();

    std::optional<std::vector<std::unique_ptr<ASTNode>>> body = std::vector<std::unique_ptr<ASTNode>>();

    while (lexer.peek().type != TOKEN_RIGHT_BRACE)
    {
        body.value().push_back(parseNext());
    }

    EXPECT_TOKEN(TOKEN_RIGHT_BRACE, "Expected '}'");
    lexer.next();

    std::optional<std::vector<std::unique_ptr<ASTNode>>> elseBody = std::vector<std::unique_ptr<ASTNode>>();

    if (lexer.peek().type == TOKEN_KEYWORD_ELSE)
    {
        lexer.next();

        EXPECT_TOKEN(TOKEN_LEFT_BRACE, "Expected '{'");
        lexer.next();

        while (lexer.peek().type != TOKEN_RIGHT_BRACE)
        {
            elseBody.value().push_back(parseNext());
        }

        EXPECT_TOKEN(TOKEN_RIGHT_BRACE, "Expected '}'");
        lexer.next();
    }

    return std::make_unique<Condition>(std::move(condition), std::move(body), std::move(elseBody));
}

std::unique_ptr<Reassign> Parser::parseReassign(std::string name)
{

    int derefCount = 0;

    while (lexer.peek().type == TOKEN_POINTER)
    {
        derefCount++;
        lexer.next();
    }

    EXPECT_TOKEN(TOKEN_OPERATOR_ASSIGN, "Expected '='");
    lexer.next();

    auto expr = parseExpression(0);

    EXPECT_TOKEN(TOKEN_SEMICOLON, "Expected ';'");
    lexer.next();

    return std::make_unique<Reassign>(std::make_unique<Variable>(name, 0), std::move(expr), derefCount);
}

std::unique_ptr<IndexReassign> Parser::parseIndexReassign(std::string name)
{
    EXPECT_TOKEN(TOKEN_LEFT_SQUARE_BRACKET, "Expected '['");
    lexer.next();

    auto index = parseExpression(0);

    EXPECT_TOKEN(TOKEN_RIGHT_SQUARE_BRACKET, "Expected ']'");
    lexer.next();

    EXPECT_TOKEN(TOKEN_OPERATOR_ASSIGN, "Expected '='");

    lexer.next();

    auto expr = parseExpression(0);

    EXPECT_TOKEN(TOKEN_SEMICOLON, "Expected ';'");
    lexer.next();

    return std::make_unique<IndexReassign>(std::make_unique<Variable>(name, 0), std::move(index), std::move(expr));
}

std::unique_ptr<VariableDeclaration> Parser::parseVariableDeclaration()
{
    auto ident = parseTypedIdent();

    if (lexer.peek().type == TOKEN_SEMICOLON)
    {
        lexer.next();
        return std::make_unique<VariableDeclaration>(std::move(ident), nullptr);
    }

    EXPECT_TOKEN(TOKEN_OPERATOR_ASSIGN, "Expected '='");
    lexer.next();

    auto expr = parseExpression(0);

    EXPECT_TOKEN(TOKEN_SEMICOLON, "Expected ';'");
    lexer.next();

    return std::make_unique<VariableDeclaration>(std::move(ident), std::move(expr));
}

std::unique_ptr<Return> Parser::parseReturn()
{

    if (lexer.peek().type == TOKEN_SEMICOLON)
    {
        lexer.next();
        return std::make_unique<Return>(nullptr);
    }

    auto expr = parseExpression(0);

    EXPECT_TOKEN(TOKEN_SEMICOLON, "Expected ';'");
    lexer.next();

    return std::make_unique<Return>(std::move(expr));
}

std::unique_ptr<Include> Parser::parseInclude()
{
    EXPECT_TOKEN(TOKEN_KEYWORD_INCLUDE, "Expected keyword include");
    lexer.next();

    EXPECT_TOKEN(TOKEN_STRING_LITERAL, "Expected string literal");
    std::string filename = lexer.peek().value;
    lexer.next();

    EXPECT_TOKEN(TOKEN_SEMICOLON, "Expected ';'");
    lexer.next();

    compiler->compile(compiler->resolvePath(filename));

    return std::make_unique<Include>(filename);
}

std::unique_ptr<FunctionDecl> Parser::parseFunctionDecl()
{

    EXPECT_TOKEN(TOKEN_IDENTIFIER, "Expected identifier");
    std::string funcName = lexer.peek().value;
    lexer.next();

    EXPECT_TOKEN(TOKEN_LEFT_PAREN, "Expected '('");
    lexer.next();

    std::vector<std::unique_ptr<TypedIdent>> args;
    while (lexer.peek().type != TOKEN_RIGHT_PAREN)
    {
        if (lexer.peek().type == TOKEN_COMMA)
        {
            lexer.next();
            continue;
        }

        args.push_back(parseTypedIdent());
    }

    EXPECT_TOKEN(TOKEN_RIGHT_PAREN, "Expected ')'");
    lexer.next();

    EXPECT_TOKEN(TOKEN_ARROW, "Expected '->'");
    lexer.next();

    auto returnType = parseType();
    auto funcDecl = std::make_unique<FunctionDecl>(funcName, args, std::move(returnType));

    if (lexer.peek().type == TOKEN_SEMICOLON)
    {
        lexer.next();
        return funcDecl;
    }

    EXPECT_TOKEN(TOKEN_LEFT_BRACE, "Expected '{'");
    lexer.next();

    std::optional<std::vector<std::unique_ptr<ASTNode>>> body = std::vector<std::unique_ptr<ASTNode>>();

    while (lexer.peek().type != TOKEN_RIGHT_BRACE)
    {
        body.value().push_back(parseNext());
    }

    funcDecl->body = std::move(body);

    EXPECT_TOKEN(TOKEN_RIGHT_BRACE, "Expected '}'");
    lexer.next();

    return funcDecl;
}

std::unique_ptr<Type> Parser::parseType()
{
    EXPECT_TOKEN(TOKEN_IDENTIFIER, "Expected identifier for type");
    Token type = lexer.peek();
    lexer.next();
    int pl = 0;

    while (lexer.peek().type == TOKEN_POINTER)
    {
        pl++;
        lexer.next();
    }

    return std::make_unique<Type>(type, pl);
}

std::unique_ptr<TypedIdent> Parser::parseTypedIdent()
{

    EXPECT_TOKEN(TOKEN_IDENTIFIER, "Expected identifier for argument");
    std::string name = lexer.peek().value;
    lexer.next();

    EXPECT_TOKEN(TOKEN_COLON, "Expected ':'");
    lexer.next();

    auto type = parseType();

    return std::make_unique<TypedIdent>(std::move(type), name);
}

std::unique_ptr<FunctionCall>
Parser::parseFunctionCall(std::string name, bool consumeSemicolon)
{
    EXPECT_TOKEN(TOKEN_LEFT_PAREN, "Expected '('");
    lexer.next();

    std::vector<std::unique_ptr<ASTNode>> args;
    while (lexer.peek().type != TOKEN_RIGHT_PAREN)
    {
        if (lexer.peek().type == TOKEN_COMMA)
        {
            lexer.next();
            continue;
        }

        args.push_back(parseExpression(0));
    }

    EXPECT_TOKEN(TOKEN_RIGHT_PAREN, "Expected ')'");
    lexer.next();

    if (consumeSemicolon)
    {
        EXPECT_TOKEN(TOKEN_SEMICOLON, "Expected ';'");
        lexer.next();
    }

    auto funcCall = std::make_unique<FunctionCall>(name, args);
    return funcCall;
}

std::unique_ptr<ASTNode>
Parser::parseExpression(int precedence = 0)
{
    auto left = parseUnary();
    for (;;)
    {
        int currentPrecedence = getPrecedence(lexer.peek().type);

        if (currentPrecedence < precedence)
        {
            break;
        }

        Token op = lexer.peek();
        lexer.next();

        auto right = parseExpression(currentPrecedence + 1);

        left = std::make_unique<BinaryExpr>(op, std::move(left), std::move(right));
    }

    return left;
}

std::unique_ptr<ASTNode>
Parser::parsePrimary()
{
    switch (lexer.peek().type)
    {
    case TOKEN_INT_LITERAL:
    {
        auto intLiteral = std::make_unique<IntLiteral>(std::stoi(lexer.peek().value));
        lexer.next();
        return intLiteral;
    }
    case TOKEN_STRING_LITERAL:
    {
        auto stringLiteral = std::make_unique<StringLiteral>(lexer.peek().value);
        lexer.next();
        return stringLiteral;
    }
    case TOKEN_BOOL_LITERAL:
    {
        auto boolLiteral = std::make_unique<BoolLiteral>(lexer.peek().value == "true");
        lexer.next();
        return boolLiteral;
    }
    case TOKEN_IDENTIFIER:
    {
        std::string name = lexer.peek().value;
        lexer.next();
        switch (lexer.peek().type)
        {
        case TOKEN_LEFT_SQUARE_BRACKET:
        {
            lexer.next();
            auto index = parseExpression(0);
            EXPECT_TOKEN(TOKEN_RIGHT_SQUARE_BRACKET, "Expected ']'");
            lexer.next();

            return std::make_unique<VariableIndex>(std::make_unique<Variable>(name, 0), std::move(index));
        }
        case TOKEN_DOT:
        {
            lexer.next();
            EXPECT_TOKEN(TOKEN_IDENTIFIER, "Expected identifier");
            std::string member = lexer.peek().value;
            lexer.next();

            return std::make_unique<StructField>(std::make_unique<Variable>(name, 0), member);
        }
        case TOKEN_LEFT_PAREN:
            return parseFunctionCall(name, false);

        default:
            return std::make_unique<Variable>(name, 0);
        }
    }
    case TOKEN_LEFT_PAREN:
    {
        lexer.next();
        auto expr = parseExpression();
        EXPECT_TOKEN(TOKEN_RIGHT_PAREN, "Expected ')'");
        lexer.next();
        return expr;
    }
    }

    return nullptr;
}

std::unique_ptr<ASTNode>
Parser::parseUnary()
{
    if (lexer.peek().type == TOKEN_OPERATOR_NOT || lexer.peek().type == TOKEN_OPERATOR_MINUS)
    {
        Token op = lexer.peek();
        lexer.next();
        auto expr = parseUnary();
        return std::make_unique<UnaryExpr>(op, std::move(expr));
    }
    return parsePrimary();
}