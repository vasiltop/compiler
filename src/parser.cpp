#include "parser.h"
#include "lexer.h"

FileParser::FileParser(std::vector<Token> tokens, std::filesystem::path path, Parser *parser) : tokens(tokens), index(0), path(path), parser(parser)
{
	baseDir = path.parent_path();
	parser->parsedFiles.insert(path);
}

Parser::Parser(std::filesystem::path p, std::filesystem::path compilerPath): compilerPath(compilerPath)
{
	if (!p.is_absolute())
	{
		p = std::filesystem::absolute(p);
	}
	
	parse(p);
}

std::filesystem::path FileParser::resolveImportPath(std::filesystem::path p)
{
	std::string asString = p; 

	if (asString.find("std:") == 0) {
		std::filesystem::path compilerDir = "/opt/compiler/";

		std::string relPath = asString.substr(4);
		std::filesystem::path fullPath = compilerDir / "std" / relPath;

		return fullPath;
	}

	return p;
}

std::vector<ASTNode *> FileParser::parse()
{
	std::vector<ASTNode *> nodes;

	expectConsume(TOKEN_KEYWORD_MODULE, "Expected keyword module");
	std::string name = expectConsume(TOKEN_STRING_LITERAL, "Expected module name").value;
	parser->pathToModule[path] = name;

	while (!eof())
	{
		if (tokens[index].type == TOKEN_KEYWORD_IMPORT)
		{
			expectConsume(TOKEN_KEYWORD_IMPORT, "Expected keyword import");
			std::filesystem::path path = expectConsume(TOKEN_STRING_LITERAL, "Expected file to import").value;
			path = resolveImportPath(path);

			if (!parser->parsedFiles.count(baseDir/path))
			{
				parser->parsedFiles.insert(baseDir/path);
				parser->parse(baseDir/path);
			}

			continue;
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

	std::cerr << path.string() << ":"
			<< pos.row << ":" << pos.col
			<< " > error: " << "Did not match any of the options when parsing global\n"
			<< " Received: " << p.value
			<< std::endl;
	exit(1);

}

FunctionDefinition *FileParser::parseFunction() {
	FunctionDefinition *def = new FunctionDefinition;
	def->moduleName = parser->pathToModule[path];

	def->name = expectConsume(TOKEN_IDENTIFIER, "Expected Global Identifier").value;
	functionSymbols.insert(def->name);

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

	std::cerr << path.string() << ":"
			<< pos.row << ":" << pos.col
			<< " > error: " << "Did not match any of the options when parsing local\n"
			<< " Received: " << p.value
			<< std::endl;
	exit(1);
}

FunctionCall *FileParser::parseFunctionCall()
{
	FunctionCall *call = new FunctionCall;

	call->moduleName = expectConsume(TOKEN_IDENTIFIER, "Provide a module for the function call").value;
	expectConsume(TOKEN_DOT, "Expected dot after module name");
	call->name = expectConsume(TOKEN_IDENTIFIER, "Provide an identifier for the function call").value;

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

		std::cerr << path.string() << ":"
			<< pos.row << ":" << pos.col
			<< " > error: " << errorMessage
			<< " Received: " << tokString
			<< std::endl;

		exit(1);
	}
}

bool FileParser::eof()
{
	return tokens[index].type == TOKEN_EOF;
}

bool Parser::isParsed(std::filesystem::path path)
{
	for (auto file: files)
	{	
		std::cout << "Comparing" << std::endl;
		std::cout << file.path << std::endl;
		std::cout << path << std::endl;
		std::cout << (file.path == path) << std::endl;

		if (file.path == path)
		{
			return true;
		}
		
	}

	return false;
}

std::set<std::string> Parser::functionSymbols(std::filesystem::path path)
{

	for (auto file: files)
	{
		if (file.path == path)
		{
			/*
			std::cout << "Function Symbols for: " << path << "\n";

			for (auto f: file.functionSymbols)
			{
				std::cout << f << "\n";
			}
			*/

			return file.functionSymbols;
		}
	}

	return {};
}

void Parser::parse(std::filesystem::path p)
{
	//std::cout << "Beginning to parse: " << p << "\n";
	Lexer lex(p);
	//lex.display(lex.tokens());

	FileParser fileParser(lex.tokens(), p, this);
	auto ast = fileParser.parse();

	/*
	std::cout << "AST for file: " << p << "\n";
	for (ASTNode *node: ast) {
		node->print(0);
	}
	std::cout << std::endl;
	*/

	FileInfo file = { p, ast, fileParser.functionSymbols };
	files.push_back(file);
}
