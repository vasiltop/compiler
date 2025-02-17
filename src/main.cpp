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
	/*
	do
	{
		token = lexer.next();
		std::cout << "Token: " << token.type << ", Value: " << token.value << std::endl;
	} while (token.type != TOKEN_EOF);
	*/

	Parser parser(lexer);

	auto ast = parser.parseFunctionDecl();
	if (!ast)
	{
		std::cerr << "Parsing failed" << std::endl;
		return 1;
	}

	llvm::LLVMContext context;
	llvm::IRBuilder<> builder(context);
	auto module = std::make_unique<llvm::Module>("my_module", context);

	// Generate LLVM IR
	ast->codegen(builder, context, *module);

	// Print the IR
	module->print(llvm::outs(), nullptr);
}
