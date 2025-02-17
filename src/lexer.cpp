#include "lexer.h"

Lexer::Lexer(const std::string &input) : input(input), position(0) {}

std::unordered_map<std::string, TokenType> Lexer::keywords = {
    {"fn", TOKEN_KEYWORD_FN},
};

size_t Lexer::getPosition()
{
    return position;
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

        switch (currentChar)
        {
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
        case ';':
            position++;
            return {TOKEN_SEMICOLON, ";"};
        default:
            position++;
            return {TOKEN_UNKNOWN, std::string(1, currentChar)};
        }
    }
    return {TOKEN_EOF, ""};
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
