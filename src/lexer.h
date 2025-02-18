#ifndef LEXER_H
#define LEXER_H

#include <string>
#include <unordered_map>

enum TokenType
{
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

    // Arithmetic
    TOKEN_OPERATOR_PLUS,
    TOKEN_OPERATOR_MINUS,
    TOKEN_OPERATOR_MUL,
    TOKEN_OPERATOR_DIV,
    TOKEN_OPERATOR_MOD,

    // Operators
    TOKEN_OPERATOR_EQUAL,
    TOKEN_OPERATOR_NOT_EQUAL,
    TOKEN_OPERATOR_LESS,
    TOKEN_OPERATOR_GREATER,
    TOKEN_OPERATOR_LESS_EQUAL,
    TOKEN_OPERATOR_GREATER_EQUAL,
    TOKEN_OPERATOR_AND,
    TOKEN_OPERATOR_OR,
    TOKEN_OPERATOR_NOT,
    TOKEN_OPERATOR_ASSIGN,

    // Literals
    TOKEN_INT_LITERAL,
    TOKEN_STRING_LITERAL,
    TOKEN_BOOL_LITERAL,
    TOKEN_CHAR_LITERAL,

    // Delimiters
    TOKEN_LEFT_BRACE,
    TOKEN_RIGHT_BRACE,
    TOKEN_LEFT_PAREN,
    TOKEN_RIGHT_PAREN,
    TOKEN_RIGHT_SQUARE_BRACKET,
    TOKEN_LEFT_SQUARE_BRACKET,

    TOKEN_IDENTIFIER,

    TOKEN_COMMA,
    TOKEN_SEMICOLON,
    TOKEN_DOT,
    TOKEN_COLON,
    TOKEN_ARROW,
    TOKEN_EOF,
    TOKEN_UNKNOWN
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
    Token peek();

    size_t getPosition();
    static std::unordered_map<std::string, TokenType> keywords;
    static std::unordered_map<TokenType, std::string> tokenEnumToString;
    void dumpTokens();

private:
    std::string input;
    size_t position;

    Token parseIdentOrKeyword();
    Token parseStringLiteral();
    Token parseInteger();
    Token parseChar();
};

#endif