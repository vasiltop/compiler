#ifndef COMPILER_H
#define COMPILER_H

#include "parser.h"

class Parser;

class Compiler
{
public:
    Compiler(int arg, char **argv);
    void compile(const std::string &filename);
    size_t amountOfFiles();

private:
    std::unordered_map<std::string, Parser> parsers;
};

#endif