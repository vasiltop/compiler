#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/IRBuilder.h"

#include "lexer.h"
#include "parser.h"

#include <string>
#include <iostream>
#include <fstream>
#include <sstream>

int main()
{
	std::ifstream file("examples/main.pl");
	std::stringstream buffer;
	buffer << file.rdbuf();

	std::string input = buffer.str();

	Token token;
	Lexer lexer(input);
	lexer.dumpTokens();
	/*
		Parser parser(lexer);
		auto ast = parser.ast();

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
