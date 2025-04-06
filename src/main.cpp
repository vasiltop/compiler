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
	std::filesystem::path compilerPath(argv[0]);

	
	if (!compilerPath.is_absolute())
		compilerPath = std::filesystem::absolute(compilerPath);
 
	std::cout << compilerPath << "\n";

	Parser par(filepath, compilerPath);
	Generator gen(&par);

	gen.generate();
}


