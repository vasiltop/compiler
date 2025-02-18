#include <iostream>

#include "lexer.h"

Lexer::Lexer(const std::string &input) : input(input), position(0) {}
/*
 // Keywords
    TOKEN_KEYWORD_FN,
    TOKEN_KEYWORD_RETURN,
    TOKEN_KEYWORD_FOR,
    TOKEN_KEYWORD_WHILE,
    TOKEN_KEYWORD_IF,
    TOKEN_KEYWORD_ELSE,
    TOKEN_KEYWORD_LET,
    TOKEN_KEYWORD_TRUE,
    TOKEN_KEYWORD_FALSE,
    TOKEN_KEYWORD_NULL,
    TOKEN_KEYWORD_STRING,
    TOKEN_KEYWORD_INT,
    TOKEN_KEYWORD_BOOL,
    TOKEN_KEYWORD_CHAR,
    TOKEN_KEYWORD_STRUCT,
*/
std::unordered_map<std::string, TokenType> Lexer::keywords = {
    {"fn", TOKEN_KEYWORD_FN},
    {"return", TOKEN_KEYWORD_RETURN},
    {"for", TOKEN_KEYWORD_FOR},
    {"while", TOKEN_KEYWORD_WHILE},
    {"if", TOKEN_KEYWORD_IF},
    {"else", TOKEN_KEYWORD_ELSE},
    {"let", TOKEN_KEYWORD_LET},
    {"true", TOKEN_BOOL_LITERAL},
    {"false", TOKEN_BOOL_LITERAL},
    {"null", TOKEN_KEYWORD_NULL},
    {"string", TOKEN_KEYWORD_STRING},
    {"int", TOKEN_KEYWORD_INT},
    {"bool", TOKEN_KEYWORD_BOOL},
    {"char", TOKEN_KEYWORD_CHAR},
    {"struct", TOKEN_KEYWORD_STRUCT}};

std::unordered_map<TokenType, std::string> tokenEnumToString = {
    {TOKEN_KEYWORD_FN, "TOKEN_KEYWORD_FN"},
    {TOKEN_KEYWORD_RETURN, "TOKEN_KEYWORD"},
    {TOKEN_KEYWORD_FOR, "TOKEN_KEYWORD_FOR"},
    {TOKEN_KEYWORD_WHILE, "TOKEN_KEYWORD_WHILE"},
    {TOKEN_KEYWORD_IF, "TOKEN_KEYWORD_IF"},
    {TOKEN_KEYWORD_ELSE, "TOKEN_KEYWORD_ELSE"},
    {TOKEN_KEYWORD_LET, "TOKEN_KEYWORD_LET"},
    {TOKEN_KEYWORD_TRUE, "TOKEN_KEYWORD_TRUE"},
    {TOKEN_KEYWORD_FALSE, "TOKEN_KEYWORD_FALSE"},
    {TOKEN_KEYWORD_NULL, "TOKEN_KEYWORD_NULL"},
    {TOKEN_KEYWORD_STRING, "TOKEN_KEYWORD_STRING"},
    {TOKEN_KEYWORD_INT, "TOKEN_KEYWORD_INT"},
    {TOKEN_KEYWORD_BOOL, "TOKEN_KEYWORD_BOOL"},
    {TOKEN_KEYWORD_CHAR, "TOKEN_KEYWORD_CHAR"},
    {TOKEN_KEYWORD_STRUCT, "TOKEN_KEYWORD_STRUCT"},
    {TOKEN_OPERATOR_PLUS, "TOKEN_OPERATOR_PLUS"},
    {TOKEN_OPERATOR_MINUS, "TOKEN_OPERATOR_MINUS"},
    {TOKEN_OPERATOR_MUL, "TOKEN_OPERATOR_MUL"},
    {TOKEN_OPERATOR_DIV, "TOKEN_OPERATOR_DIV"},
    {TOKEN_OPERATOR_MOD, "TOKEN_OPERATOR_MOD"},
    {TOKEN_OPERATOR_EQUAL, "TOKEN_OPERATOR_EQUAL"},
    {TOKEN_OPERATOR_NOT_EQUAL, "TOKEN_OPERATOR_NOT_EQUAL"},
    {TOKEN_OPERATOR_LESS, "TOKEN_OPERATOR_LESS"},
    {TOKEN_OPERATOR_GREATER, "TOKEN_OPERATOR_GREATER"},
    {TOKEN_OPERATOR_LESS_EQUAL, "TOKEN_OPERATOR_LESS_EQUAL"},
    {TOKEN_OPERATOR_GREATER_EQUAL, "TOKEN_OPERATOR_GREATER_EQUAL"},
    {TOKEN_OPERATOR_AND, "TOKEN_OPERATOR_AND"},
    {TOKEN_OPERATOR_OR, "TOKEN_OPERATOR_OR"},
    {TOKEN_OPERATOR_NOT, "TOKEN_OPERATOR_NOT"},
    {TOKEN_OPERATOR_ASSIGN, "TOKEN_OPERATOR_ASSIGN"},
    // give the rest at once
    {TOKEN_INT_LITERAL, "TOKEN_INT_LITERAL"},
    {TOKEN_STRING_LITERAL, "TOKEN_STRING_LITERAL"},
    {TOKEN_BOOL_LITERAL, "TOKEN_BOOL_LITERAL"},
    {TOKEN_CHAR_LITERAL, "TOKEN_CHAR_LITERAL"},
    {TOKEN_LEFT_BRACE, "TOKEN_LEFT_BRACE"},
    {TOKEN_RIGHT_BRACE, "TOKEN_RIGHT_BRACE"},
    {TOKEN_LEFT_PAREN, "TOKEN_LEFT_PAREN"},
    {TOKEN_RIGHT_PAREN, "TOKEN_RIGHT_PAREN"},
    {TOKEN_RIGHT_SQUARE_BRACKET, "TOKEN_RIGHT_SQUARE_BRACKET"},
    {TOKEN_LEFT_SQUARE_BRACKET, "TOKEN_LEFT_SQUARE_BRACKET"},
    {TOKEN_IDENTIFIER, "TOKEN_IDENTIFIER"},
    {TOKEN_COMMA, "TOKEN_COMMA"},
    {TOKEN_SEMICOLON, "TOKEN_SEMICOLON"},
    {TOKEN_DOT, "TOKEN_DOT"},
    {TOKEN_COLON, "TOKEN_COLON"},
    {TOKEN_ARROW, "TOKEN_ARROW"},
    {TOKEN_EOF, "TOKEN_EOF"},
    {TOKEN_UNKNOWN, "TOKEN_UNKNOWN"}};

void Lexer::dumpTokens()
{
    Token token;
    do
    {
        token = next();
        std::cout << "Token: " << tokenEnumToString[token.type] << " Value: " << token.value << std::endl;
    } while (token.type != TOKEN_EOF);
}

size_t Lexer::getPosition()
{
    return position;
}

Token Lexer::peek()
{
    size_t oldPosition = position;
    Token token = next();
    position = oldPosition;
    return token;
}

Token Lexer::next()
{
    while (position < input.size())
    {
        char currentChar = input[position];

        if (std::isspace(currentChar))
        {
            position++;
            continue;
        }

        if (std::isalpha(currentChar))
        {
            return parseIdentOrKeyword();
        }

        if (currentChar == '"')
        {
            return parseStringLiteral();
        }

        if (std::isdigit(currentChar))
        {
            return parseInteger(); // TODO: add floats
        }

        if (currentChar == '\'')
        {
            return parseChar();
        }

        switch (currentChar)
        {

        // Arithmetic operators
        case '+':
            position++;
            return {TOKEN_OPERATOR_PLUS, "+"};
        case '-':
            position++;

            if (position < input.size() && input[position] == '>')
            {
                position++;
                return {TOKEN_ARROW, "->"};
            }
            return {TOKEN_OPERATOR_MINUS, "-"};
        case '*':
            position++;
            return {TOKEN_OPERATOR_MUL, "*"};
        case '/':
            position++;
            return {TOKEN_OPERATOR_DIV, "/"};
        case '%':
            position++;
            return {TOKEN_OPERATOR_MOD, "%"};

        // Operators
        case '=':
            position++;

            if (position < input.size() && input[position] == '=')
            {
                position++;
                return {TOKEN_OPERATOR_EQUAL, "=="};
            }
            return {TOKEN_OPERATOR_ASSIGN, "="};
        case '<':
            position++;

            if (position < input.size() && input[position] == '=')
            {
                position++;
                return {TOKEN_OPERATOR_LESS_EQUAL, "<="};
            }

            return {TOKEN_OPERATOR_LESS, "<"};
        case '>':
            position++;

            if (position < input.size() && input[position] == '=')
            {
                position++;
                return {TOKEN_OPERATOR_GREATER_EQUAL, ">="};
            }
            return {TOKEN_OPERATOR_GREATER, ">"};
        case '&':
            position++;
            if (position < input.size() && input[position] == '&')
            {
                position++;
                return {TOKEN_OPERATOR_AND, "&&"};
            }
            return {TOKEN_UNKNOWN, "&"}; // TODO: add TOKEN_REF
        case '|':
            position++;
            if (position < input.size() && input[position] == '|')
            {
                position++;
                return {TOKEN_OPERATOR_OR, "||"};
            }
            return {TOKEN_UNKNOWN, "|"};
        case '!':
            position++;

            if (position < input.size() && input[position] == '=')
            {
                position++;
                return {TOKEN_OPERATOR_NOT_EQUAL, "!="};
            }
            return {TOKEN_OPERATOR_NOT, "!"};

        // Delimiters
        case '{':
            position++;
            return {TOKEN_LEFT_BRACE, "{"};
        case '}':
            position++;
            return {TOKEN_RIGHT_BRACE, "}"};
        case '(':
            position++;
            return {TOKEN_LEFT_PAREN, "("};
        case ')':
            position++;
            return {TOKEN_RIGHT_PAREN, ")"};
        case '[':
            position++;
            return {TOKEN_LEFT_SQUARE_BRACKET, "["};
        case ']':
            position++;
            return {TOKEN_RIGHT_SQUARE_BRACKET, "]"};

        case ';':
            position++;
            return {TOKEN_SEMICOLON, ";"};
        case ',':
            position++;
            return {TOKEN_COMMA, ","};
        case '.':
            position++;
            return {TOKEN_DOT, "."};
        case ':':
            position++;
            return {TOKEN_COLON, ":"};
        default:
            position++;
            return {TOKEN_UNKNOWN, std::string(1, currentChar)};
        }
    }
    return {TOKEN_EOF, "EOF"};
}

Token Lexer::parseIdentOrKeyword()
{
    std::string lexeme;
    while (position < input.size() && (std::isalnum(input[position]) || input[position] == '_'))
    {
        lexeme += input[position];
        position++;
    }

    if (keywords.find(lexeme) != keywords.end())
    {
        return {keywords[lexeme], lexeme};
    }

    return {TOKEN_IDENTIFIER, lexeme};
}

Token Lexer::parseStringLiteral()
{
    std::string literal;
    position++;

    while (position < input.size() && input[position] != '"')
    {
        literal += input[position];
        position++;
    }

    if (position < input.size() && input[position] == '"')
    {
        position++;
    }

    return {TOKEN_STRING_LITERAL, literal};
}

Token Lexer::parseInteger()
{
    std::string number;
    while (position < input.size() && std::isdigit(input[position]))
    {
        number += input[position];
        position++;
    }

    return {TOKEN_INT_LITERAL, number};
}

Token Lexer::parseChar()
{
    std::string character;
    position++;

    if (position < input.size())
    {
        character += input[position];
        position++;
    }

    if (position < input.size() && input[position] == '\'')
    {
        position++;
    }

    return {TOKEN_CHAR_LITERAL, character};
}