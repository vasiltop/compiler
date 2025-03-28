#ifndef PARSER_H
#define PARSER_H

#include <filesystem>
#include <optional>
#include <iostream>
#include "lexer.h"

struct ASTNode {};

struct Type : public ASTNode
{
	size_t pointerLevel;
	std::string name;
};

struct FunctionDefinition : public ASTNode
{
	std::string name;
	std::vector<std::string> paramNames;
	std::vector<Type> paramTypes;
	Type returnType;
	std::optional<std::vector<ASTNode>> body;
};

struct FunctionCall : public ASTNode
{
	std::string name;
	std::vector<ASTNode> params;
};

struct FileInfo
{
	std::filesystem::path path;
	std::vector<ASTNode> nodes;
};

class Parser 
{
public:
	Parser(std::filesystem::path path);
	void parse(std::filesystem::path path);

private:
	ASTNode parseGlobal();
	bool eof();
	void expect(TokenType type, std::string errorMessage);
	void expectConsume(TokenType type, std::string errorMessage);
	std::vector<ASTNode> parse();

	std::vector<FileInfo> files;

	// Current parse data
	size_t index;
	std::vector<Token> tokens;
	std::filesystem::path path;
};

#endif
