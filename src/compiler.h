#ifndef COMPILER_H
#define COMPILER_H

#include "parser.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/IRBuilder.h"

class Parser;

class Compiler
{
public:
    Compiler(int arg, char **argv);
    void compile(const std::string &filename);
    size_t amountOfFiles();

private:
    std::unordered_map<std::string, Parser> parsers;
    std::unique_ptr<llvm::LLVMContext> context;
    std::unique_ptr<llvm::IRBuilder<>> builder;
    std::unique_ptr<llvm::Module> module;
};

#endif