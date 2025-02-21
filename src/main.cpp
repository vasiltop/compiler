#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/IRBuilder.h"

#include "lexer.h"
#include "parser.h"
#include "compiler.h"
#include <string>
#include <iostream>

int main(int argc, char **argv)
{

	Compiler compiler(argc, argv);

	/*
llvm::LLVMContext context;
llvm::IRBuilder<> builder(context);
auto module = std::make_unique<llvm::Module>("my_module", context);

for (auto &node : ast)
{
	node->codegen(builder, context, *module);
}

module->print(llvm::outs(), nullptr);
*/
}
