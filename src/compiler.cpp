#include "compiler.h"
#include <iostream>

fs::path Compiler::resolvePath(const std::string &filename) const
{
    fs::path filepath(filename);

    if (filepath.is_absolute())
    {
        return filepath;
    }

    return baseDir / filepath;
}

Compiler::Compiler(int argc, char **argv)
{
    if (argc < 2)
    {
        std::cerr << "Usage: " << argv[0] << " <filename>" << std::endl;
        exit(1);
    }

    char *inputPath = argv[1];
    fs::path filepath(inputPath);

    if (!filepath.is_absolute())
    {
        filepath = fs::absolute(filepath);
    }

    baseDir = filepath.parent_path();

    context = std::make_unique<llvm::LLVMContext>();
    builder = std::make_unique<llvm::IRBuilder<>>(*context);
    module = std::make_unique<llvm::Module>("main", *context);

    compile(resolvePath(filepath));
    module->print(llvm::outs(), nullptr);
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
    std::cout << "AST for " << filename << std::endl;
    for (auto &node : ast)
    {
        node->display(0);
    }
    std::cout << std::endl;
    */

    for (auto &node : ast)
    {
        node->codegen(*builder, *module, parser);
    }
}

size_t Compiler::amountOfFiles()
{
    return parsers.size();
}
