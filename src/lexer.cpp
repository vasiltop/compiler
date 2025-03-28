#include <iostream>

#include "lexer.h"
#include <fstream>
#include <sstream>

std::string getContents(const char *filename)
{
    std::ifstream file(filename);
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

InputBuffer::InputBuffer() : position(0), positionInFile({1, 0}) {}

InputBuffer::InputBuffer(std::filesystem::path filename) : filename(filename)
{
    std::string contents = getContents(filename.c_str());
    buffer = contents;
    position = 0;
    positionInFile.row = 1;
    positionInFile.col = 0;
}

size_t InputBuffer::size()
{
    return buffer.size();
}

char InputBuffer::advance()
{
    if (position >= buffer.size())
    {
        return '\0';
    }

    char currentChar = buffer[position];
    position++;
    positionInFile.col++;

    if (currentChar == '\n')
    {
        positionInFile.row++;
        positionInFile.col = 0;
    }

    return currentChar;
}

char InputBuffer::current()
{
    if (position >= buffer.size())
    {
        return '\0';
    }

    return buffer[position];
}

void InputBuffer::skipWhitespace()
{
    while (position < buffer.size() && std::isspace(buffer[position]))
    {
        advance();
    }
}

void InputBuffer::skipComment()
{
    if (position + 1 < buffer.size() && buffer[position] == '/' && buffer[position + 1] == '/')
    {
        while (position < buffer.size() && buffer[position] != '\n')
        {
            advance();
        }
        skipWhitespace();
        skipComment();
    }
}

bool InputBuffer::eof()
{
    return position >= buffer.size();
}

Lexer::Lexer(std::filesystem::path filename)
{
	if (!filename.is_absolute())
	{
		filename = std::filesystem::absolute(filename);
	}
	input = InputBuffer(filename);
}
 
std::unordered_map<std::string, TokenType> Lexer::keywords = {
    {"return", TOKEN_KEYWORD_RETURN},
    {"for", TOKEN_KEYWORD_FOR},
    {"while", TOKEN_KEYWORD_WHILE},
    {"if", TOKEN_KEYWORD_IF},
    {"else", TOKEN_KEYWORD_ELSE},
    {"true", TOKEN_BOOL_LITERAL},
    {"false", TOKEN_BOOL_LITERAL},
    {"null", TOKEN_KEYWORD_NULL},
    {"struct", TOKEN_KEYWORD_STRUCT},
    {"import", TOKEN_KEYWORD_IMPORT}};

std::unordered_map<TokenType, std::string> Lexer::tokenEnumToString = {
    {TOKEN_KEYWORD_RETURN, "TOKEN_KEYWORD"},
    {TOKEN_KEYWORD_FOR, "TOKEN_KEYWORD_FOR"},
    {TOKEN_KEYWORD_WHILE, "TOKEN_KEYWORD_WHILE"},
    {TOKEN_KEYWORD_IF, "TOKEN_KEYWORD_IF"},
    {TOKEN_KEYWORD_ELSE, "TOKEN_KEYWORD_ELSE"},
    {TOKEN_KEYWORD_TRUE, "TOKEN_KEYWORD_TRUE"},
    {TOKEN_KEYWORD_FALSE, "TOKEN_KEYWORD_FALSE"},
    {TOKEN_KEYWORD_NULL, "TOKEN_KEYWORD_NULL"},
    {TOKEN_KEYWORD_STRUCT, "TOKEN_KEYWORD_STRUCT"},
    {TOKEN_KEYWORD_IMPORT, "TOKEN_KEYWORD_IMPORT"},
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
    {TOKEN_HASHTAG, "TOKEN_HASHTAG"},
    {TOKEN_SEMICOLON, "TOKEN_SEMICOLON"},
    {TOKEN_POINTER, "TOKEN_POINTER"},
    {TOKEN_DOT, "TOKEN_DOT"},
    {TOKEN_COLON, "TOKEN_COLON"},
    {TOKEN_ARROW, "TOKEN_ARROW"},
    {TOKEN_EOF, "TOKEN_EOF"},
    {TOKEN_UNKNOWN, "TOKEN_UNKNOWN"}};

void Lexer::display(std::vector<Token> tokens)
{
    std::cout << "Tokens for file: " << input.filename << std::endl;

		for (size_t i = 0; i < tokens.size(); ++i)
		{
			Token token = tokens[i];
		 	std::cout << "Token: " << tokenEnumToString[token.type] << " Value: " << token.value
                  << " Position: Row: " << token.position.row << " Column: " << token.position.col << std::endl;
		}
    
    std::cout << std::endl;

    input.position = 0;
    input.positionInFile = {1, 0};
}

std::vector<Token> Lexer::tokens()
{
	std::vector<Token> tokens;
	Token token;
	do
	{
		token = next();
		tokens.push_back(token);
	} while (token.type != TOKEN_EOF);

	return tokens;
}

Token Lexer::next()
{

    while (!input.eof())
    {
        input.skipWhitespace();
        input.skipComment();

        if (input.eof())
        {
            return {TOKEN_EOF, "EOF", input.positionInFile};
        }

        char currentChar = input.current();

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

        FilePosition position = input.positionInFile;

        switch (currentChar)
        {

        // Arithmetic operators
        case '+':
            input.advance();
            return {TOKEN_OPERATOR_PLUS, "+", position};
        case '-':

            input.advance();
            if (input.current() == '>')
            {
                input.advance();
                return {TOKEN_ARROW, "->", position};
            }
            return {TOKEN_OPERATOR_MINUS, "-", position};
        case '*':
            input.advance();
            return {TOKEN_OPERATOR_MUL, "*", position};
        case '/':
            input.advance();
            return {TOKEN_OPERATOR_DIV, "/", position};
        case '%':
            input.advance();
            return {TOKEN_OPERATOR_MOD, "%", position};

        // Operators
        case '=':
            input.advance();

            if (input.current() == '=')
            {
                input.advance();
                return {TOKEN_OPERATOR_EQUAL, "==", position};
            }

            return {TOKEN_OPERATOR_ASSIGN, "=", position};
        case '<':

            input.advance();

            if (input.current() == '=')
            {
                input.advance();
                return {TOKEN_OPERATOR_LESS_EQUAL, "<=", position};
            }

            return {TOKEN_OPERATOR_LESS, "<", position};
        case '>':
            input.advance();

            if (input.current() == '=')
            {
                input.advance();
                return {TOKEN_OPERATOR_GREATER_EQUAL, ">=", position};
            }

            return {TOKEN_OPERATOR_GREATER, ">", position};
        case '&':
            input.advance();
            if (input.current() == '&')
            {
                input.advance();
                return {TOKEN_OPERATOR_AND, "&&", position};
            }
            return {TOKEN_UNKNOWN, "&", position}; // TODO: ref
        case '|':
            input.advance();
            if (input.current() == '|')
            {
                input.advance();
                return {TOKEN_OPERATOR_OR, "||", position};
            }
            return {TOKEN_UNKNOWN, "|", position};
        case '!':
            input.advance();
            if (input.current() == '=')
            {
                input.advance();
                return {TOKEN_OPERATOR_NOT_EQUAL, "!=", position};
            }
            return {TOKEN_OPERATOR_NOT, "!", position};

        // Delimiters
        case '{':
            input.advance();
            return {TOKEN_LEFT_BRACE, "{", position};

        case '}':
            input.advance();
            return {TOKEN_RIGHT_BRACE, "}", position};
        case '(':
            input.advance();
            return {TOKEN_LEFT_PAREN, "(", position};
        case ')':
            input.advance();
            return {TOKEN_RIGHT_PAREN, ")", position};
        case '[':
            input.advance();
            return {TOKEN_LEFT_SQUARE_BRACKET, "[", position};
        case ']':
            input.advance();
            return {TOKEN_RIGHT_SQUARE_BRACKET, "]", position};

        case ';':
            input.advance();
            return {TOKEN_SEMICOLON, ";", position};
        case '^':
            input.advance();
            return {TOKEN_POINTER, "^", position};
        case ',':
            input.advance();
            return {TOKEN_COMMA, ",", position};
        case '.':
            input.advance();
            return {TOKEN_DOT, ".", position};
        case '#':
            input.advance();
            return {TOKEN_HASHTAG, "#", position};
        case ':':
            input.advance();
            return {TOKEN_COLON, ":", position};
        default:
            input.advance();
            return {TOKEN_UNKNOWN, std::string(1, currentChar), position};
        }
    }
    return {TOKEN_EOF, "EOF", input.positionInFile};
}

Token Lexer::peek()
{
    size_t position = input.position;
    FilePosition filePosition = input.positionInFile;

    Token token = next();

    input.position = position;
    input.positionInFile = filePosition;

    return token;
}

Token Lexer::parseIdentOrKeyword()
{
    std::string lexeme;
    FilePosition position = input.positionInFile;

    while (!input.eof() && (std::isalnum(input.current()) || input.current() == '_'))
    {
        lexeme += input.current();
        input.advance();
    }

    if (keywords.find(lexeme) != keywords.end())
    {
        return {keywords[lexeme], lexeme, position};
    }

    return {TOKEN_IDENTIFIER, lexeme, position};
}

Token Lexer::parseStringLiteral()
{
    std::string literal;
    FilePosition position = input.positionInFile;
    input.advance();

    while (!input.eof() && input.current() != '"')
    {

        if (input.current() == '\\')
        {

            input.advance();

            if (input.eof())
            {
                break;
            }

            switch (input.current())
            {
            case 'n':
                literal += '\n';
                break;
            case 't':
                literal += '\t';
                break;
            case 'r':
                literal += '\r';
                break;
            case '\\':
                literal += '\\';
                break;
            case '"':
                literal += '"';
                break;
            default:
                literal += input.current();
                break;
            }

            input.advance();
            continue;
        }

        literal += input.current();
        input.advance();
    }

    if (!input.eof() && input.current() == '"')
    {
        input.advance();
    }

    return {TOKEN_STRING_LITERAL, literal, position};
}

Token Lexer::parseInteger()
{
    std::string number;
    FilePosition position = input.positionInFile;

    while (!input.eof() && std::isdigit(input.current()))
    {
        number += input.current();
        input.advance();
    }

    return {TOKEN_INT_LITERAL, number, position};
}

Token Lexer::parseChar()
{
    std::string character;
    FilePosition position = input.positionInFile;
    input.advance();

    if (!input.eof())
    {
        character += input.current();
        input.advance();
    }

    if (!input.eof() && input.current() == '\'')
    {
        input.advance();
    }

    return {TOKEN_CHAR_LITERAL, character, position};
}
