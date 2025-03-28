#include "parser.h"
#include "lexer.h"

Parser::Parser(std::filesystem::path p): index(0)
{
	if (!p.is_absolute())
	{
		p = std::filesystem::absolute(p);
	}

	baseDir = p.parent_path();
	
	parse(p);
}

std::vector<ASTNode> Parser::parse()
{
	std::vector<ASTNode> nodes;
	
	while (index < tokens.size())
	{
		// Check if we need to import another file

		if (tokens[index].type == TOKEN_HASHTAG) {
			index++;
			
			expectConsume(TOKEN_KEYWORD_IMPORT, "Expected keyword import");

			expect(TOKEN_STRING_LITERAL, "Expected file to import");
			std::filesystem::path path = tokens[index].value;
			index++;

			parse(baseDir/path);

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
		case TOKEN_IDENTIFIER:
				return parseFunction();
		break;
	}

	std::cerr << "Did not match any of the options when parsing global\n";
	exit(1);

}

ASTNode Parser::parseFunction() {
	FunctionDefinition def;
	def.name = expectConsume(TOKEN_IDENTIFIER, "Expected Global Identifier").value;

	expectConsume(TOKEN_COLON, "Expected Global Definition (::)");
	expectConsume(TOKEN_COLON, "Expected Global Definition (::)");

	expectConsume(TOKEN_LEFT_PAREN, "Expected opening function paren");

	// Parse arguments
	
	while (tokens[index].type != TOKEN_RIGHT_PAREN)
	{
		if (tokens[index].type == TOKEN_COMMA)
		{
			index++;
			continue;
		}
		
		Token varName = expectConsume(TOKEN_IDENTIFIER, "Expected variable name");
		def.paramNames.push_back(varName.value);
		
		expectConsume(TOKEN_COLON, "Expected colon after type");

		def.paramTypes.push_back(parseType());
	}

	expectConsume(TOKEN_RIGHT_PAREN, "Expected closing function paren");

	expect(TOKEN_IDENTIFIER, "Expected return type");
	def.returnType = parseType();

	if (tokens[index].type == TOKEN_LEFT_BRACE)
	{
		std::vector<ASTNode> body;
		index++;

		while (tokens[index].type != TOKEN_RIGHT_BRACE)
		{
			body.push_back(parseLocal());
		}

		def.body = body;

		expectConsume(TOKEN_RIGHT_BRACE, "Expected closing function brace");
	}

	return def;
}

Type Parser::parseType()
{
	Type t;
	t.pointerLevel = 0;
	
	while (tokens[index].type == TOKEN_POINTER)
	{
		t.pointerLevel++;
		index++;
	}
	
	auto tok = expectConsume(TOKEN_IDENTIFIER, "Expected type identifier");
	t.name = tok.value;

	return t;
}

ASTNode Parser::parseLocal()
{
	switch (tokens[index].type)
	{
		case TOKEN_IDENTIFIER:
			return parseFunctionCall();

		case TOKEN_KEYWORD_RETURN:
			Return ret;
			expectConsume(TOKEN_KEYWORD_RETURN, "Expected the return keyword");
			auto tok = expectConsume(TOKEN_INT_LITERAL, "Expected the return keyword");
			ret.value = std::stoi(tok.value);

			return ret;
	}

	Token p = tokens[index];
	FilePosition pos = p.position;
	std::cerr << path.string() << ":"                                   \
			<< pos.row << ":" << pos.col \
			<< " > error: " << "Did not match any of the options when parsing local\n" \
			<< " Received: " << p.value                           \
			<< std::endl;
	exit(1);
}

ASTNode Parser::parseFunctionCall()
{
	FunctionCall call;

	auto tok = expectConsume(TOKEN_IDENTIFIER, "Provide an identifier for the function call");
	call.name = tok.value;

	
	expectConsume(TOKEN_LEFT_PAREN, "Expected opening function paren");

	// Parse arguments

	/*
	while (tokens[index].type != TOKEN_RIGHT_PAREN)
	{
		if (tokens[index].type == TOKEN_COMMA)
		{
			index++;
			continue;
		}
		
		auto tok = expectConsume(TOKEN_IDENTIFIER, "Expected variable name");
		paramNames.push_back(tok.value);
		
		expectConsume(TOKEN_COLON, "Expected colon after type");

		auto tok = expect(TOKEN_IDENTIFIER, "Expected variable type");
		paramTypes.push_back(tok.value);
	}
	*/

	expectConsume(TOKEN_RIGHT_PAREN, "Expected closing function paren");
	expectConsume(TOKEN_SEMICOLON, "Expected a semicolon");

	return call;
}

Token Parser::expectConsume(TokenType type, std::string errorMessage)
{
	expect(type, errorMessage);
	auto tok = tokens[index];
	index++;
	return tok;
}

void Parser::expect(TokenType type, std::string errorMessage)
{
	if (eof() || tokens[index].type != type) 
	{
		Token p = tokens[index];
		FilePosition pos = p.position;

		std::cerr << path.string() << ":"                                   \
			<< pos.row << ":" << pos.col \
			<< " > error: " << errorMessage                                   \
			<< " Received: " << p.value                           \
			<< std::endl;

		exit(1);
	}
}

bool Parser::eof()
{
	return tokens[index + 1].type == TOKEN_EOF;
}

void Parser::parse(std::filesystem::path p)
{
	this->path = p;
	index = 0;

	Lexer lex(path);
	tokens = lex.tokens();
	lex.display(tokens);

	auto ast = parse();

	FileInfo file = { path, ast };
	files.push_back(file);
}
