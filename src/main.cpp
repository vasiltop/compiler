#include <filesystem>
#include <iostream>
#include "lexer.h"
#include "parser.h"
#include "generator.h"

int main(int argc, char** argv) 
{
	if (argc < 2) 
	{
			std::cerr << "Usage: " << argv[0] << " <filename>" << std::endl;
			exit(1);
	}

	char* inputPath = argv[1];
	std::filesystem::path filepath(inputPath);
 
	Parser par(filepath);

	Generator gen(par);
	gen.generate();
}


