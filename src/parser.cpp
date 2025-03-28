#include "parser.h"
#include "lexer.h"

Parser::Parser(std::filesystem::path path): index(0)
{
	parse(path);
}

std::vector<ASTNode> Parser::parse()
{
	std::vector<ASTNode> nodes;
	
	while (index < tokens.size())
	{
		// Check if we need to import another file

		if tokens[index].type == TOKEN_HASHTAG {
			expectConsume(TOKEN_KEYWORD_IMPORT, "Expected Import");
			expect(TOKEN_IDENTIFIER, "Expected file to import");
			std::string file = tokens[index].value;
			index++;

			continue;
		}

		nodes.push_back(parseGlobal());
	}

	return nodes;
}

ASTNode Parser::parseGlobal()
{
	Token cur = tokens[index];

	switch (cur.type)
	{
	}
}

void Parser::expectConsume(TokenType type, std::string errorMessage)
{
	expect(type, errorMessage);
	index++;
}

void Parser::expect(TokenType type, std::string errorMessage)
{
	if (eof() || tokens[index + 1].type != type) 
	{
		Token p = tokens[index + 1];
		FilePosition pos = p.position;

		std::cerr << path.string() << ":"                                   \
			<< pos.row << ":" << pos.col \
			<< " > error: " << errorMessage                                   \
			<< " Received: " << p.value                           \
			<< std::endl;
	}
}

bool Parser::eof()
{
	return tokens[index + 1].type == TOKEN_EOF;
}

void Parser::parse(std::filesystem::path path)
{
	Lexer lex(path);
	tokens = lex.tokens();
	lex.display(tokens);

	auto ast = parse();
	this->path = path;
	index = 0;

	FileInfo file = { path, ast };
	files.push_back(file);
}
