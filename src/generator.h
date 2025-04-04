#ifndef GENERATOR_H
#define GENERATOR_H

#include "parser.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/Support/raw_ostream.h"
#include <map>

struct GType
{
	llvm::Type *elementType;
	size_t depth;

	llvm::Type *type(llvm::LLVMContext &ctx);
};

class Generator
{
public:
	Generator(Parser parser);
	void generate();

private:
	Parser parser;
	llvm::LLVMContext ctx;
	llvm::IRBuilder<> builder;
	llvm::Module module;
	
	std::map<std::string, llvm::Function *> functions;
	GType typeInfo(Type *type);
};

#endif
