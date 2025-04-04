#include "parser.h"
#include "lexer.h"

FileParser::FileParser(std::vector<Token> tokens, std::filesystem::path path, Parser *parser) : tokens(tokens), index(0), path(path), parser(parser)
{
	baseDir = path.parent_path();
}

Parser::Parser(std::filesystem::path p)
{
	if (!p.is_absolute())
	{
		p = std::filesystem::absolute(p);
	}
	
	parse(p);
}

std::vector<ASTNode *> FileParser::parse()
{
	std::vector<ASTNode *> nodes;
	
	while (!eof())
	{
		// Check if we need to import another file

		if (tokens[index].type == TOKEN_HASHTAG) {
			index++;
			
			expectConsume(TOKEN_KEYWORD_IMPORT, "Expected keyword import");

			expect(TOKEN_STRING_LITERAL, "Expected file to import");
			std::filesystem::path path = tokens[index].value;
			index++;

			parser->parse(baseDir/path);
		}

		nodes.push_back(parseGlobal());
	}

	return nodes;
}

ASTNode *FileParser::parseGlobal()
{
	Token cur = tokens[index];

	switch (cur.type)
	{
		case TOKEN_IDENTIFIER:
				return parseFunction();
		break;
	}

	Token p = tokens[index];
	FilePosition pos = p.position;

	std::cerr << path.string() << ":"                                   \
			<< pos.row << ":" << pos.col \
			<< " > error: " << "Did not match any of the options when parsing global\n" \
			<< " Received: " << p.value                           \
			<< std::endl;
	exit(1);

}

FunctionDefinition *FileParser::parseFunction() {
	FunctionDefinition *def = new FunctionDefinition;

	def->name = expectConsume(TOKEN_IDENTIFIER, "Expected Global Identifier").value;

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
		def->paramNames.push_back(varName.value);
		
		expectConsume(TOKEN_COLON, "Expected colon after type");

		def->paramTypes.push_back(parseType());
	}

	expectConsume(TOKEN_RIGHT_PAREN, "Expected closing function paren");

	expect(TOKEN_IDENTIFIER, "Expected return type");
	def->returnType = parseType();

	if (tokens[index].type == TOKEN_LEFT_BRACE)
	{
		std::vector<ASTNode *> body;
		index++;

		while (tokens[index].type != TOKEN_RIGHT_BRACE)
		{
			body.push_back(parseLocal());
		}

		def->body = body;

		expectConsume(TOKEN_RIGHT_BRACE, "Expected closing function brace");
	}

	return def;
}

Type *FileParser::parseType()
{
	Type *t = new Type;
	t->pointerLevel = 0;
	
	while (tokens[index].type == TOKEN_POINTER)
	{
		t->pointerLevel++;
		index++;
	}
	
	auto tok = expectConsume(TOKEN_IDENTIFIER, "Expected type identifier");
	t->name = tok.value;

	return t;
}

ASTNode *FileParser::parseLocal()
{
	switch (tokens[index].type)
	{
		case TOKEN_IDENTIFIER:
			return parseFunctionCall();

		case TOKEN_KEYWORD_RETURN:
			Return *ret = new Return;
			expectConsume(TOKEN_KEYWORD_RETURN, "Expected the return keyword");
			auto tok = expectConsume(TOKEN_INT_LITERAL, "Expected the return keyword");
			ret->value = std::stoi(tok.value);
			expectConsume(TOKEN_SEMICOLON, "Expected semicolon after return");

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

FunctionCall *FileParser::parseFunctionCall()
{
	FunctionCall *call = new FunctionCall;

	auto tok = expectConsume(TOKEN_IDENTIFIER, "Provide an identifier for the function call");
	call->name = tok.value;

	
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
	auto stringToken = expectConsume(TOKEN_STRING_LITERAL, "Expected string literal");

	StringLiteral *s = new StringLiteral;
	s->value = stringToken.value;

	call->params.push_back(s);

	expectConsume(TOKEN_RIGHT_PAREN, "Expected closing function paren");
	expectConsume(TOKEN_SEMICOLON, "Expected a semicolon");

	return call;
}

Token FileParser::expectConsume(TokenType type, std::string errorMessage)
{
	expect(type, errorMessage);
	auto tok = tokens[index];
	index++;
	return tok;
}

void FileParser::expect(TokenType type, std::string errorMessage)
{
	if (eof() || tokens[index].type != type) 
	{
		Token p = tokens[index];
		FilePosition pos = p.position;
		std::string tokString = Lexer::tokenEnumToString[p.type];

		std::cerr << path.string() << ":"                                   \
			<< pos.row << ":" << pos.col \
			<< " > error: " << errorMessage                                   \
			<< " Received: " << tokString \
			<< std::endl;

		exit(1);
	}
}

bool FileParser::eof()
{
	return tokens[index].type == TOKEN_EOF;
}

void Parser::parse(std::filesystem::path p)
{
	Lexer lex(p);
	lex.display(lex.tokens());

	FileParser fileParser(lex.tokens(), p, this);
	auto ast = fileParser.parse();

	std::cout << "AST for file: " << p << "\n";

	for (ASTNode *node: ast) {
		node->print(0);
	}

	std::cout << std::endl;

	FileInfo file = { p, ast };
	files.push_back(file);
}
