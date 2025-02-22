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
}
