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

void writeModuleToFile(std::unique_ptr<llvm::Module> module, const std::string &filename)
{
    std::error_code errorCode;
    llvm::raw_fd_ostream outFile(filename, errorCode);

    if (errorCode)
    {
        llvm::errs() << "Error opening file '" << filename << "': " << errorCode.message() << "\n";
        return;
    }

    module->print(outFile, nullptr);

    outFile.close();
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
    writeModuleToFile(std::move(module), "build/output.ll");
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

    for (auto &node : ast)
    {
        node->codegen(*builder, *module, parser);
    }
}

size_t Compiler::amountOfFiles()
{
    return parsers.size();
}
