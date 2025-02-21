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
    parse(filename);
}

void Compiler::parse(const std::string &filename)
{
    Parser parser(this, filename);
    parsers.insert({filename, parser});
    auto ast = parser.parse();

    for (auto &node : ast)
    {
        std::cout << "AST for: " << filename << std::endl;
        node->display(0);
    }
}

size_t Compiler::amountOfFiles()
{
    return parsers.size();
}
