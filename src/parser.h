#ifndef PARSER_H
#define PARSER_H

#include <filesystem>
#include <optional>
#include <iostream>
#include "lexer.h"

struct ASTNode
{
	virtual ~ASTNode() = default;
	virtual void print(int level)
	{
		std::cout << "Unimplemented\n";
	}
};

static void indentPrint(int indent, const std::string &str)
{
	std::string indentStr(indent * 2, ' ');
	std::cout << indentStr << str << std::endl;
}

struct Type : public ASTNode
{
	size_t pointerLevel;
	std::string name;

	void print(int level) override
	{
		indentPrint(level, "Type:");
		indentPrint(level + 1, "Level: " + std::to_string(pointerLevel));
		indentPrint(level + 1, "Name: " + name);
	}
};

struct StringLiteral : public ASTNode
{
	std::string value;

	void print(int level) override
	{
		indentPrint(level, "String: " + value);
	}
};

struct FunctionDefinition : public ASTNode
{
	std::string name;
	std::vector<std::string> paramNames;
	std::vector<Type *> paramTypes;
	Type *returnType;
	std::optional<std::vector<ASTNode *>> body;

	void print(int level) override
	{
		indentPrint(level, "Function: " + name);
		indentPrint(level + 1, "Return Type:");
		returnType->print(level + 2);
		indentPrint(level + 1, "Parameters:");
		for (size_t i = 0; i < paramNames.size(); i++)
		{
			indentPrint(level + 2, "Name: " + paramNames[i]);
			indentPrint(level + 2, "Type:");
			paramTypes[i]->print(level + 3);
		}
		if (body.has_value())
		{
			indentPrint(level + 1, "Body:");
			for (auto node : body.value())
			{
				node->print(level + 2);
			}
		}
	}
};

struct FunctionCall : public ASTNode
{
	std::string name;
	std::vector<ASTNode *> params;

	void print(int level) override
	{
		indentPrint(level, "Function Call: " + name);
		indentPrint(level + 1, "Parameters:");
		for (auto param : params)
		{
			param->print(level + 2);
		}
	}
};

struct Return : public ASTNode
{
	int value;

	void print(int level) override
	{
		indentPrint(level, "Return: " + std::to_string(value));
	}
};

struct FileInfo
{
	std::filesystem::path path;
	std::vector<ASTNode *> nodes;
};

class Parser
{
public:
	Parser(std::filesystem::path path);
	void parse(std::filesystem::path path);

private:
	std::vector<FileInfo> files;
};

class FileParser
{
public:
	FileParser(std::vector<Token> tokens, std::filesystem::path path, Parser *parser);
	std::vector<ASTNode *> parse();

private:
	Parser *parser;
	bool eof();
	void expect(TokenType type, std::string errorMessage);
	Token expectConsume(TokenType type, std::string errorMessage);

	size_t index;
	std::vector<Token> tokens;
	std::filesystem::path path;
	std::filesystem::path baseDir;

	// Node parsers
	ASTNode *parseGlobal();
	ASTNode *parseLocal();
	FunctionDefinition *parseFunction();
	FunctionCall *parseFunctionCall();
	Type *parseType();
};

#endif
