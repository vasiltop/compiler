#ifndef LEXER_H
#define LEXER_H

#include <string>
#include <unordered_map>

enum TokenType
{
    TOKEN_KEYWORD_FN,     // "fn"
    TOKEN_IDENTIFIER,     // "main"
    TOKEN_LEFT_BRACE,     // "{"
    TOKEN_RIGHT_BRACE,    // "}"
    TOKEN_LEFT_PAREN,     // "("
    TOKEN_RIGHT_PAREN,    // ")"
    TOKEN_STRING_LITERAL, // "Hello, world!"
    TOKEN_SEMICOLON,      // ";"
    TOKEN_EOF,            // End of input
    TOKEN_UNKNOWN         // Unknown token
};

struct Token
{
    TokenType type;
    std::string value;
};

class Lexer
{
public:
    Lexer(const std::string &input);
    Token next();
    size_t getPosition();
    static std::unordered_map<std::string, TokenType> keywords;

private:
    std::string input;
    size_t position;

    Token parseIdentOrKeyword();
    Token parseStringLiteral();
};

#endif