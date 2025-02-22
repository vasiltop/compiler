#include "compiler.h"
#include <iostream>

Compiler::Compiler(int arg, char **argv)
{
    if (arg < 2)
    {
        std::cerr << "Usage: " << argv[0] << " <filename>" << std::endl;
        exit(1);
    }

    char *filename = argv[1];
    compile(filename);
}

void Compiler::compile(const std::string &filename)
{
    if (parsers.find(filename) != parsers.end())
    {
        return;
    }

    Parser parser(this, filename);
    parsers.insert({filename, parser});

    auto ast = parser.parse();

    /*
    for (auto &node : ast)
    {
        node->display(0);
    }

    */
    llvm::LLVMContext context;
    llvm::IRBuilder<> builder(context);
    auto module = std::make_unique<llvm::Module>(filename, context);
    (*module).setSourceFileName(filename);

    for (auto &node : ast)
    {
        node->codegen(builder, *module, parser);
    }

    module->print(llvm::outs(), nullptr);
}

size_t Compiler::amountOfFiles()
{
    return parsers.size();
}
