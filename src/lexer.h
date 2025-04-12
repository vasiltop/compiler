#ifndef LEXER_H
#define LEXER_H

#include <filesystem>
#include <string>
#include <vector>
#include <unordered_map>

enum TokenType
{
    // Keywords
    TOKEN_KEYWORD_RETURN,
    TOKEN_KEYWORD_EXTERN,
    TOKEN_KEYWORD_FOR,
    TOKEN_KEYWORD_WHILE,
    TOKEN_KEYWORD_LET,
    TOKEN_KEYWORD_IF,
    TOKEN_KEYWORD_ELSE,
    TOKEN_KEYWORD_TRUE,
    TOKEN_KEYWORD_FALSE,
    TOKEN_KEYWORD_NULL,

    TOKEN_KEYWORD_STRUCT,
    TOKEN_KEYWORD_IMPORT,
    TOKEN_KEYWORD_MODULE,
 
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
    TOKEN_HASHTAG,
		TOKEN_AT,
    TOKEN_SEMICOLON,
    TOKEN_POINTER,
		TOKEN_REFERENCE,
    TOKEN_DOT,
    TOKEN_COLON,
    TOKEN_ARROW,
    TOKEN_EOF,
    TOKEN_UNKNOWN
};

struct FilePosition
{
    size_t row;
    size_t col;
};

struct Token
{
    TokenType type;
    std::string value;
    FilePosition position;
};

class InputBuffer
{

public:
    InputBuffer();
    InputBuffer(std::filesystem::path filename);

    size_t position;
    FilePosition positionInFile;
    size_t size();
    char advance();
    char current();
    bool eof();
    void skipWhitespace();
    void skipComment();
		std::filesystem::path filename;
    std::string buffer;
};

class Lexer
{
public:
    Lexer(std::filesystem::path filename);

    Token next();
    Token peek();

    static std::unordered_map<std::string, TokenType> keywords;
    static std::unordered_map<TokenType, std::string> tokenEnumToString;
    void display(std::vector<Token> tokens);
		std::vector<Token> tokens();

    InputBuffer input;

private:
    Token parseIdentOrKeyword();
    Token parseStringLiteral();
    Token parseInteger();
    Token parseChar();
};

#endif
