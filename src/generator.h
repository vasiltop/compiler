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

struct GType
{
	llvm::Type *elementType;
	size_t depth;

	llvm::Type *type(llvm::LLVMContext &ctx);
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
private:
	Parser *parser;
	GType typeInfo(Type *type);
	void generateFunctionDefinitions();
};

#endif
