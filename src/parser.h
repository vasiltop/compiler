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
class GScope;

static void indentPrint(int indent, const std::string &str)
{
	std::string indentStr(indent * 2, ' ');
	std::cout << indentStr << str << std::endl;
}

struct ASTNode
{
	virtual ~ASTNode() = default;
	virtual void print(int level)
	{
		std::cout << "Unimplemented\n";
	}

	virtual llvm::Value* codegen(GScope *scope, Generator *gen)
	{
		return nullptr;
	}

};

struct Block: public ASTNode
{
	std::vector<ASTNode *> body;

	llvm::Value* codegen(GScope *scope, Generator *gen) override;
	Block(std::vector<ASTNode *> body)
    : body(body) {}

	void print(int level) override
	{
		indentPrint(level, "Block: ");
		for (auto &l: body)
		{
			l->print(level + 2);
		}
	}
};

struct Type : public ASTNode
{
	size_t pointerLevel;
	std::string name;

	Type(size_t pointerLevel, std::string name) : pointerLevel(pointerLevel), name(name) {}

	void print(int level) override
	{
		indentPrint(level, "Type:");
		indentPrint(level + 2, "Level: " + std::to_string(pointerLevel));
		indentPrint(level + 2, "Name: " + name);
	}

	bool isSigned()
	{
		if (
				name == "u8" || 
				name == "u16" || 
				name == "u32" || 
				name == "u64"
				)
		{
			return false;
		}

		return true;
	}
};

struct ArrayType : public Type
{
	Type* type;
	int size;

	ArrayType(Type *type, int size, size_t pointerLevel): Type(pointerLevel, ""), type(type), size(size) {}

	void print(int level) override
	{
		indentPrint(level, "Array Type:");
		indentPrint(level + 2, "Level: " + std::to_string(pointerLevel));
		type->print(level+2);
	}
};

struct StructType : public Type
{
	std::string moduleName;

	StructType(std::string moduleName, std::string structName, size_t pointerLevel) : Type(pointerLevel, structName), moduleName(moduleName) {}

	void print(int level) override
	{
		indentPrint(level, "Struct Type:");
		indentPrint(level + 2, "Level: " + std::to_string(pointerLevel));
		indentPrint(level + 2, "Module: " + moduleName);
		indentPrint(level + 2, "Name: " + name);
	}
};

struct FunctionDefinition : public ASTNode
{
	std::string moduleName;
	std::string name;
	std::vector<std::string> paramNames;
	std::vector<Type *> paramTypes;
	Type *returnType;
	
	Block *body; // could be nullptr if no body

	llvm::Value* codegen(GScope *scope, Generator *gen) override;
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
		if (body)
		{
			indentPrint(level + 1, "Body:");
			body->print(level + 2);
		}
	}
};

struct FunctionCall : public ASTNode
{
	std::string moduleName;
	std::string name;
	std::vector<ASTNode *> params;

	llvm::Value* codegen(GScope *scope, Generator *gen) override;
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

struct StructDefinition : public ASTNode
{
	std::string name;
	std::string moduleName;
	std::vector<std::string> fieldNames;
	std::vector<Type *> fieldTypes;
	

	// llvm::Value* codegen(GScope *scope, Generator *gen) override;
	StructDefinition(std::string name, std::string moduleName, std::vector<std::string> fieldNames, std::vector<Type *> fieldTypes) : name(name), moduleName(moduleName), fieldNames(fieldNames), fieldTypes(fieldTypes) {}
		
	void print(int level) override
	{
		indentPrint(level, "Struct Decl: " + name);

		for (size_t i = 0; i < fieldNames.size(); ++i)
		{
			indentPrint(level + 2, "Field name: " + fieldNames[i]);
			fieldTypes[i]->print(level + 2);
		}
	}
};

struct Return : public ASTNode
{
	ASTNode *expr;

	llvm::Value* codegen(GScope *scope, Generator *gen) override;
	void print(int level) override
	{
		indentPrint(level, "Return: ");
		expr->print(level + 1);
	}
};

struct Assign: public ASTNode
{
	ASTNode *lhs;
	ASTNode *rhs;

	Assign(ASTNode *lhs, ASTNode *rhs): lhs(lhs), rhs(rhs) {}
	llvm::Value* codegen(GScope *scope, Generator *gen) override;
	void print(int level) override
	{
		indentPrint(level, "Assign: ");
		lhs->print(level + 1);
		rhs->print(level + 1);
	}
};

struct ArrayLiteral: public ASTNode
{
	std::vector<ASTNode *> values;

	llvm::Value* codegen(GScope *scope, Generator *gen) override;
	ArrayLiteral(std::vector<ASTNode *> values) : values(values) {}
	void print(int level) override
	{
		indentPrint(level, "ArrayLiteral: ");

		for (auto &v: values)
		{
			v->print(level + 2);
		}
	}
};


struct StringLiteral: public ASTNode
{
	std::string value;

	llvm::Value* codegen(GScope *scope, Generator *gen) override;
	StringLiteral(const std::string& val) : value(val) {}
	void print(int level) override
	{
		indentPrint(level, "StringLiteral: " + value);
	}
};

struct CharLiteral: public ASTNode
{
	char value;

	llvm::Value* codegen(GScope *scope, Generator *gen) override;
	CharLiteral(char value) : value(value) {}
	void print(int level) override
	{
		indentPrint(level, "CharLiteral: " + value);
	}
};

struct Variable: public ASTNode
{
	std::string name;

	llvm::Value* codegen(GScope *scope, Generator *gen) override;
	Variable(const std::string& name) : name(name) {}
	void print(int level) override
	{
		indentPrint(level, "Variable: " + name);
	}
};

struct VariableAccess: public ASTNode
{
	std::string varName;
	std::vector<ASTNode *> indexes;

	llvm::Value* codegen(GScope *scope, Generator *gen) override;
	VariableAccess(std::string varName, std::vector<ASTNode *> indexes) : varName(varName), indexes(indexes) {}

	void print(int level) override
	{
		indentPrint(level, "Variable Access: " + varName);

		for (auto &index: indexes)
		{
			index->print(level + 2);
		}
	}
};

struct StructLiteral : public ASTNode
{
	std::string moduleName;
	std::string name;
	std::vector<std::string> fieldNames;
	std::vector<ASTNode *> fieldExprs;

	//llvm::Value* codegen(GScope *scope, Generator *gen) override;
	StructLiteral(std::string moduleName, std::string name, std::vector<std::string> fieldNames, std::vector<ASTNode *> fieldExprs) : moduleName(moduleName), name(name), fieldNames(fieldNames), fieldExprs(fieldExprs) {}

	void print(int level) override
	{
		indentPrint(level, "Struct Literal: " + name);
		indentPrint(level + 2, "Module: " + moduleName);

		for (size_t i = 0; i < fieldNames.size(); ++i)
		{
			indentPrint(level + 2, "Field: " + fieldNames[i]);
			fieldExprs[i]->print(level + 2);
		}
	}
};

struct StructField : public ASTNode
{
	std::string fieldName;

	//llvm::Value* codegen(GScope *scope, Generator *gen) override;
	StructField(std::string fieldName) : fieldName(fieldName) {}
	void print(int level) override
	{
		indentPrint(level, "Struct Field: " + fieldName);
	}
};

struct ArrayIndex: public ASTNode
{
	ASTNode *expr;

	//llvm::Value* codegen(GScope *scope, Generator *gen) override;
	ArrayIndex(ASTNode *expr) : expr(expr) {}
	void print(int level) override
	{
		indentPrint(level, "Array Index: ");
		expr->print(level + 2);
	}
};

struct VariableDecl: public ASTNode
{
	std::string varName;
	Type *type;
	ASTNode *expr;

	llvm::Value* codegen(GScope *scope, Generator *gen) override;
	VariableDecl(const std::string& varName, Type *type, ASTNode *expr) : varName(varName), type(type), expr(expr) {}

	void print(int level) override
	{
		indentPrint(level, "Variable Decl: " + varName);
		type->print(level + 2);
		expr->print(level + 2);
	}
};

struct IntLiteral: public ASTNode
{
	int value;

	llvm::Value* codegen(GScope *scope, Generator *gen) override;
  IntLiteral(int val) : value(val) {}
	void print(int level) override
	{
		indentPrint(level, "IntLiteral: " + std::to_string(value));
	}
};

struct BoolLiteral: public ASTNode
{
	bool value;

	llvm::Value* codegen(GScope *scope, Generator *gen) override;
	BoolLiteral(bool val) : value(val) {}
	void print(int level) override
	{
		indentPrint(level, "BoolLiteral: " + std::to_string(value));
	}
};

struct BinaryExpr : public ASTNode
{
	Token op;
	ASTNode *lhs;
	ASTNode *rhs;

	llvm::Value* codegen(GScope *scope, Generator *gen) override;
	BinaryExpr(Token op, ASTNode* left, ASTNode* right)
		: op(op), lhs(left), rhs(right) {}
	void print(int level) override
	{
		indentPrint(level, "BinaryExpr: " + op.value);
		lhs->print(level + 2);
		rhs->print(level + 2);
	}
};

struct UnaryExpr: public ASTNode
{
	Token op;
	ASTNode *expr;

	llvm::Value* codegen(GScope *scope, Generator *gen) override;
	UnaryExpr(Token op, ASTNode* expr)
    : op(op), expr(expr) {}
	void print(int level) override
	{
		indentPrint(level, "UnaryExpr: " + op.value);
		expr->print(level + 2);
	}
};

struct Cast: public ASTNode
{
	Type *type;
	ASTNode *expr;

	llvm::Value* codegen(GScope *scope, Generator *gen) override;
	Cast(Type *type, ASTNode *expr)
    : type(type), expr(expr) {}
	void print(int level) override
	{
		indentPrint(level, "Cast: ");
		type->print(level + 2);
		expr->print(level + 2);
	}
};

struct While: public ASTNode
{
	ASTNode* condition;
	Block* body;

	llvm::Value* codegen(GScope *scope, Generator *gen) override;
	While(ASTNode *condition, Block *body): condition(condition), body(body) {}

	void print(int level) override
	{
		indentPrint(level, "While: ");
		condition->print(level + 2);
		body->print(level + 2);
	}
};

struct Conditional: public ASTNode
{
	std::vector<std::pair<ASTNode *, Block *>> conditions; // condition and block

	llvm::Value* codegen(GScope *scope, Generator *gen) override;
	Conditional(std::vector<std::pair<ASTNode *, Block *>> conditions) 
    : conditions(conditions) {}

	void print(int level) override
	{
		indentPrint(level, "Conditional: ");

		for (auto &condition: conditions)
		{
			if (condition.first)
			{
				indentPrint(level, "Condition: ");
				condition.first->print(level + 2);
			} else {

				indentPrint(level, "Else: ");
			}

			indentPrint(level, "Block: ");
			condition.second->print(level + 2);
		}

	}
};

struct FileInfo
{
	std::filesystem::path path;
	std::vector<ASTNode *> nodes;
	std::set<std::string> functionSymbols;
	std::set<std::string> structSymbols;
};

class Parser
{
public:
	Parser(std::filesystem::path path, std::filesystem::path compilerPath);

	void parse(std::filesystem::path path);
	bool isParsed(std::filesystem::path path);

	std::vector<FileInfo> files;
	std::set<std::string> parsedFiles;
	std::map<std::filesystem::path, std::string> pathToModule;
	std::filesystem::path compilerPath;
};

class FileParser
{
public:
	FileParser(std::vector<Token> tokens, std::filesystem::path path, Parser *parser);
	std::vector<ASTNode *> parse();
	std::set<std::string> functionSymbols;
	std::set<std::string> structSymbols;

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
	ASTNode *parseExpression(int precedence = 0);
	ASTNode *parseUnary();
	ASTNode *parsePrimary();
	ASTNode *parseSpecial();
	FunctionDefinition *parseFunction();
	StructDefinition *parseStruct();
	Assign *parseAssign();
	FunctionCall *parseFunctionCall();
	Conditional *parseConditional();
	Block *parseBlock();
	While *parseWhile();
	StructLiteral *parseStructLiteral();
	VariableDecl *parseVariableDecl();
	Type *parseType();
};

#endif
