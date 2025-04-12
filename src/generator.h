#ifndef GENERATOR_H
#define GENERATOR_H

#include "parser.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/Support/raw_ostream.h"
#include <map>

class FileInfo;
class Parser;
class Type;
class ASTNode;

struct GType
{
	llvm::Type *elementType;
	size_t depth;

	llvm::Type *type(llvm::LLVMContext &ctx);
	bool isPointer() const { return depth > 0; }
};

struct GScope
{
	GScope *parent;
	std::map<std::string, std::pair<llvm::Value *, GType>> variables;
	
	GScope(GScope *parent);

	std::pair<llvm::Value *, GType> getVar(std::string name);
};

class Generator
{
public:
	Generator(Parser *parser);
	void generate();

	llvm::LLVMContext ctx;
	llvm::IRBuilder<> builder;
	llvm::Module module;
	FileInfo *currentFile;

	void displayFunctionSymbols();
	std::map<std::string, std::map<std::string, llvm::Function *>> functionSymbols;
	GType typeInfo(Type *type);
	GType expressionType(ASTNode *node, GScope *scope);

	bool inReferenceContext = false;

private:
	Parser *parser;
	void generateFunctionDefinitions();
};

#endif
