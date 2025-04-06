#ifndef PARSER_H
#define PARSER_H

#include <filesystem>
#include <optional>
#include <iostream>
#include <set>
#include <map>

#include "lexer.h"
#include "generator.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/IRBuilder.h"

class Generator;

struct ASTNode
{
	virtual ~ASTNode() = default;
	virtual void print(int level)
	{
		std::cout << "Unimplemented\n";
	}

	virtual llvm::Value* codegen(Generator *gen)
	{
		return nullptr;
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
	std::string moduleName;
	std::string name;
	std::vector<std::string> paramNames;
	std::vector<Type *> paramTypes;
	Type *returnType;
	std::optional<std::vector<ASTNode *>> body;

	llvm::Value* codegen(Generator *gen) override;
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
	std::string moduleName;
	std::string name;
	std::vector<ASTNode *> params;

	llvm::Value* codegen(Generator *gen) override;
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

	llvm::Value* codegen(Generator *gen) override;
	void print(int level) override
	{
		indentPrint(level, "Return: " + std::to_string(value));
	}
};

struct FileInfo
{
	std::filesystem::path path;
	std::vector<ASTNode *> nodes;
	std::set<std::string> functionSymbols;
};

class Parser
{
public:
	Parser(std::filesystem::path path, std::filesystem::path compilerPath);

	void parse(std::filesystem::path path);
	bool isParsed(std::filesystem::path path);

	std::vector<FileInfo> files;
	std::set<std::string> parsedFiles;
	std::set<std::string> functionSymbols(std::filesystem::path path);
	std::map<std::filesystem::path, std::string> pathToModule;
	std::filesystem::path compilerPath;

};

class FileParser
{
public:
	FileParser(std::vector<Token> tokens, std::filesystem::path path, Parser *parser);
	std::vector<ASTNode *> parse();
	std::set<std::string> functionSymbols;

private:
	Parser *parser;
	bool eof();
	void expect(TokenType type, std::string errorMessage);
	Token expectConsume(TokenType type, std::string errorMessage);
	std::filesystem::path resolveImportPath(std::filesystem::path p);

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
