#ifndef COMPILER_H
#define COMPILER_H

#include "parser.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/IRBuilder.h"
#include <filesystem>

namespace fs = std::filesystem;

class Parser;

class Compiler
{
public:
    Compiler(int arg, char **argv);
    void compile(const std::string &filename);
    size_t amountOfFiles();
    std::string baseDir;

    fs::path resolvePath(const std::string &filename) const;

private:
    std::unordered_map<std::string, Parser> parsers;
    std::unique_ptr<llvm::LLVMContext> context;
    std::unique_ptr<llvm::IRBuilder<>> builder;
    std::unique_ptr<llvm::Module> module;
};

#endif