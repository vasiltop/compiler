#include "parser.h"
#include "lexer.h"

FileParser::FileParser(std::vector<Token> tokens, std::filesystem::path path, Parser *parser) : tokens(tokens), index(0), path(path), parser(parser)
{
	baseDir = path.parent_path();
	parser->parsedFiles.insert(path);
}

Parser::Parser(std::filesystem::path p, std::filesystem::path compilerPath) : compilerPath(compilerPath)
{
	if (!p.is_absolute())
	{
		p = std::filesystem::absolute(p);
	}

	parse(p);
}

std::filesystem::path FileParser::resolveImportPath(std::filesystem::path p)
{
	std::string asString = p.string();
	asString += ".pl";

	if (asString.find("std:") == 0)
	{
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

			if (!parser->parsedFiles.count(baseDir / path))
			{
				parser->parsedFiles.insert(baseDir / path);
				parser->parse(baseDir / path);
			}

			continue;
		}

		auto node = parseGlobal();
		node->print(0);
		nodes.push_back(node);
	}

	return nodes;
}

ASTNode *FileParser::parseGlobal()
{
	Token cur = tokens[index];

	switch (cur.type)
	{
	case TOKEN_IDENTIFIER:
		if (tokens[index + 3].type == TOKEN_LEFT_PAREN)
			return parseFunction();
		else if (tokens[index + 3].type == TOKEN_KEYWORD_STRUCT)
			return parseStruct();
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

StructDefinition *FileParser::parseStruct()
{
	auto name = expectConsume(TOKEN_IDENTIFIER, "").value;
	auto moduleName = parser->pathToModule[path];

	structSymbols.insert(name);

	std::vector<std::string> fieldNames;
	std::vector<Type *> fieldTypes;

	expectConsume(TOKEN_COLON, "Expected colon after name");
	expectConsume(TOKEN_COLON, "Expected colon after name");

	expectConsume(TOKEN_KEYWORD_STRUCT, "");

	expectConsume(TOKEN_LEFT_BRACE, "Expected colon after name");

	while (tokens[index].type != TOKEN_RIGHT_BRACE)
	{
		fieldNames.push_back(expectConsume(TOKEN_IDENTIFIER, "").value);
		expectConsume(TOKEN_COLON, "Expected colon after name");
		fieldTypes.push_back(parseType());

		if (tokens[index].type == TOKEN_COMMA)
			expectConsume(TOKEN_COMMA, "Expected comma after field");
	}

	expectConsume(TOKEN_RIGHT_BRACE, "Expected colon after name");

	return new StructDefinition(name, moduleName, fieldNames, fieldTypes);
}

FunctionDefinition *FileParser::parseFunction()
{

	FunctionDefinition *def = new FunctionDefinition;
	def->body = nullptr;
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

		std::string varName = expectConsume(TOKEN_IDENTIFIER, "Expected variable name").value;
		def->paramNames.push_back(varName);
		expectConsume(TOKEN_COLON, "Expected colon after type");
		Type *type = parseType();
		def->paramTypes.push_back(type);
	}

	expectConsume(TOKEN_RIGHT_PAREN, "Expected closing function paren");

	expect(TOKEN_IDENTIFIER, "Expected return type");
	def->returnType = parseType();

	if (tokens[index].type == TOKEN_LEFT_BRACE)
	{
		def->body = parseBlock();
	}

	return def;
}

Type *FileParser::parseType()
{
	Type *t = new Type(0, "");
	t->pointerLevel = 0;

	while (tokens[index].type == TOKEN_POINTER)
	{
		t->pointerLevel++;
		index++;
	}

	if (tokens[index].type == TOKEN_LEFT_SQUARE_BRACKET)
	{
		index++;
		auto arrayType = parseType();
		expectConsume(TOKEN_SEMICOLON, "Expected semicolon in array type");
		auto size = std::stoi(expectConsume(TOKEN_INT_LITERAL, "Expected array size").value);
		expectConsume(TOKEN_RIGHT_SQUARE_BRACKET, "Expected closing bracket");

		return new ArrayType(arrayType, size, t->pointerLevel);
	}

	auto name = expectConsume(TOKEN_IDENTIFIER, "Expected type identifier").value;

	if (tokens[index].type == TOKEN_COLON)
	{
		index++;
		auto structName = expectConsume(TOKEN_IDENTIFIER, "Expected type identifier").value;

		return new StructType(name, structName, t->pointerLevel);
	}
	else if (!isBuiltInType(name))
	{
		return new StructType(parser->pathToModule[path], name, t->pointerLevel);
	}

	t->name = name;

	return t;
}

bool FileParser::isBuiltInType(std::string &t)
{
	std::vector<std::string> types = {
		"i64",
		"u64",
		"i32",
		"u32",
		"i16",
		"u16",
		"i8",
		"u8",
		"f64",
		"f32",
		"bool",
		"void",
		"string",
		"char"};

	return std::find(types.begin(), types.end(), t) != types.end();
}

Assign *FileParser::parseAssign()
{
	auto lhs = parseExpression();
	/*
	auto name = expectConsume(TOKEN_IDENTIFIER, "");
	*/
	expectConsume(TOKEN_OPERATOR_ASSIGN, "");
	auto rhs = parseExpression();
	expectConsume(TOKEN_SEMICOLON, "Expect semicolon");

	return new Assign(lhs, rhs);
}

Block *FileParser::parseBlock()
{
	expectConsume(TOKEN_LEFT_BRACE, "Expected block brace");
	std::vector<ASTNode *> body;

	while (tokens[index].type != TOKEN_RIGHT_BRACE)
	{
		body.push_back(parseLocal());
	}

	expectConsume(TOKEN_RIGHT_BRACE, "");

	return new Block(body);
}

While *FileParser::parseWhile()
{
	expectConsume(TOKEN_KEYWORD_WHILE, "");
	auto condition = parseExpression();
	auto body = parseBlock();

	return new While(condition, body);
}

Conditional *FileParser::parseConditional()
{
	std::vector<std::pair<ASTNode *, Block *>> conditions;

	do
	{
		ASTNode *cond = nullptr;

		if (tokens[index].type == TOKEN_KEYWORD_IF)
		{
			index++;
			cond = parseExpression();
		}

		auto block = parseBlock();
		conditions.push_back({cond, block});

	} while (tokens[index++].type == TOKEN_KEYWORD_ELSE);

	index--;

	return new Conditional(conditions);
}

ASTNode *FileParser::parseLocal()
{
	switch (tokens[index].type)
	{
	case TOKEN_POINTER:
	{
		return parseAssign();
	}
	case TOKEN_IDENTIFIER:
	{
		if (tokens[index + 1].type == TOKEN_LEFT_PAREN)
		{
			auto f = parseFunctionCall(parser->pathToModule[path]);
			expectConsume(TOKEN_SEMICOLON, "Expected semicolon");
			return f;
		}
		if (tokens[index + 1].type == TOKEN_OPERATOR_ASSIGN || tokens[index + 1].type == TOKEN_LEFT_SQUARE_BRACKET || tokens[index + 1].type == TOKEN_DOT)
		{
			return parseAssign();
		}

		if (tokens[index + 1].type == TOKEN_COLON)
		{
			auto module = tokens[index].value;
			index += 2;
			FunctionCall *f = parseFunctionCall(module);
			expectConsume(TOKEN_SEMICOLON, "Expected semicolon");
			return f;
		}
	}
	case TOKEN_KEYWORD_RETURN:
	{
		Return *ret = new Return;
		expectConsume(TOKEN_KEYWORD_RETURN, "Expected the return keyword");
		ret->expr = parseExpression();
		expectConsume(TOKEN_SEMICOLON, "Expected semicolon after return");

		return ret;
	}
	case TOKEN_KEYWORD_LET:
		return parseVariableDecl();
	case TOKEN_LEFT_BRACE:
		return parseBlock();
	case TOKEN_KEYWORD_IF:
		return parseConditional();
	case TOKEN_KEYWORD_WHILE:
		return parseWhile();
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

FunctionCall *FileParser::parseFunctionCall(std::string &module)
{
	FunctionCall *call = new FunctionCall;

	call->moduleName = module;
	call->name = expectConsume(TOKEN_IDENTIFIER, "Provide an identifier for the function call").value;

	expectConsume(TOKEN_LEFT_PAREN, "Expected opening function paren");

	// Parse arguments
	while (tokens[index].type != TOKEN_RIGHT_PAREN)
	{
		if (tokens[index].type == TOKEN_COMMA)
		{
			index++;
			continue;
		}

		call->params.push_back(parseExpression());
	}

	expectConsume(TOKEN_RIGHT_PAREN, "Expected closing function paren");

	return call;
}

VariableDecl *FileParser::parseVariableDecl()
{
	expectConsume(TOKEN_KEYWORD_LET, "");
	auto name = expectConsume(TOKEN_IDENTIFIER, "Expected variable name").value;
	expectConsume(TOKEN_COLON, "Expect colon for variable type");
	auto type = parseType();
	expectConsume(TOKEN_OPERATOR_ASSIGN, "Expect assign eq");

	if (dynamic_cast<ArrayType *>(type))
	{
		std::cout << "AAAAAAAAAAAAAAAAAAA\n";
	}
	auto expr = parseExpression();
	expectConsume(TOKEN_SEMICOLON, "Expected semicolon");

	return new VariableDecl(name, type, expr);
}

int getPrecedence(TokenType type)
{
	switch (type)
	{
	case TOKEN_OPERATOR_MUL:
	case TOKEN_OPERATOR_DIV:
	case TOKEN_OPERATOR_MOD:
		return 5;
	case TOKEN_OPERATOR_PLUS:
	case TOKEN_OPERATOR_MINUS:
		return 4;
	case TOKEN_OPERATOR_LESS:
	case TOKEN_OPERATOR_GREATER:
	case TOKEN_OPERATOR_LESS_EQUAL:
	case TOKEN_OPERATOR_GREATER_EQUAL:
		return 3;
	case TOKEN_OPERATOR_NOT_EQUAL:
	case TOKEN_OPERATOR_EQUAL:
		return 2;
	case TOKEN_OPERATOR_AND:
		return 1;
	case TOKEN_OPERATOR_OR:
		return 0;
	default:
		return -1;
	}
}

ASTNode *FileParser::parseExpression(int precedence)
{
	auto left = parseUnary();

	for (;;)
	{
		Token tok = tokens[index];
		int currentPrecedence = getPrecedence(tok.type);

		if (currentPrecedence < precedence)
		{
			break;
		}

		index++;

		auto right = parseExpression(currentPrecedence + 1);

		left = new BinaryExpr(tok, left, right);
	}

	return left;
}

ASTNode *FileParser::parseUnary()
{
	Token tok = tokens[index];

	if (tok.type == TOKEN_REFERENCE || tok.type == TOKEN_OPERATOR_NOT || tok.type == TOKEN_OPERATOR_MINUS || tok.type == TOKEN_POINTER)
	{
		index++;
		auto expr = parseUnary();
		return new UnaryExpr(tok, expr);
	}

	return parsePrimary();
}

ASTNode *FileParser::parsePrimary()
{
	auto cur = tokens[index];
	auto module = parser->pathToModule[path];
	index++;

	switch (cur.type)
	{
	case TOKEN_INT_LITERAL:
		return new IntLiteral(std::stoi(cur.value));
	case TOKEN_STRING_LITERAL:
		return new StringLiteral(cur.value);
	case TOKEN_BOOL_LITERAL:
		return new BoolLiteral(cur.value == "true");
	case TOKEN_CHAR_LITERAL:
		return new CharLiteral(cur.value[0]);
	case TOKEN_AT:
		return parseSpecial();
	case TOKEN_LEFT_PAREN:
	{
		auto expr = parseExpression();
		expectConsume(TOKEN_RIGHT_PAREN, "Expected ) after parsing expression");
		return expr;
	}
	case TOKEN_IDENTIFIER:
	{
		// Struct Literal with implicit module
		if (tokens[index].type == TOKEN_LEFT_BRACE)
		{
			index--;
			return parseStructLiteral(module);
		}
		else if (tokens[index].type == TOKEN_LEFT_PAREN) // Function call with implicit module
		{
			index--;
			return parseFunctionCall(module);
		}
		if (tokens[index].type == TOKEN_COLON)
		{
			index++;
			expectConsume(TOKEN_IDENTIFIER, "Expected struct or function name.");
			if (tokens[index].type == TOKEN_LEFT_PAREN)
			{
				index--;
				return parseFunctionCall(cur.value);
			}
			else if (tokens[index].type == TOKEN_LEFT_BRACE)
			{
				index--;
				return parseStructLiteral(cur.value);
			}
		}

		std::vector<ASTNode *> indexes;

		while (tokens[index].type == TOKEN_LEFT_SQUARE_BRACKET || tokens[index].type == TOKEN_DOT)
		{
			switch (tokens[index].type)
			{
			case TOKEN_LEFT_SQUARE_BRACKET:
				expectConsume(TOKEN_LEFT_SQUARE_BRACKET, "Expected left square bracket");
				indexes.push_back(new ArrayIndex(parseExpression()));
				expectConsume(TOKEN_RIGHT_SQUARE_BRACKET, "Expected right square bracket");
				break;
			case TOKEN_DOT:
				expectConsume(TOKEN_DOT, "");
				indexes.push_back(new StructField(expectConsume(TOKEN_IDENTIFIER, "Expected Identifier").value));
				break;
			}
		}

		if (indexes.size())
			return new VariableAccess(cur.value, indexes);

		return new Variable(cur.value);
	}
	case TOKEN_LEFT_SQUARE_BRACKET:
	{
		std::vector<ASTNode *> values;
		do
		{
			values.push_back(parseExpression());

			if (tokens[index].type == TOKEN_COMMA)
				index++;

		} while (tokens[index].type != TOKEN_RIGHT_SQUARE_BRACKET);

		index++;

		return new ArrayLiteral(values);
	}
	}

	return nullptr;
}

StructLiteral *FileParser::parseStructLiteral(std::string &module)
{
	auto moduleName = module;
	auto name = expectConsume(TOKEN_IDENTIFIER, "Expected ident").value;

	expectConsume(TOKEN_LEFT_BRACE, "Expected left square bracket");

	std::vector<std::string> fieldNames;
	std::vector<ASTNode *> fieldExprs;

	while (tokens[index].type != TOKEN_RIGHT_BRACE)
	{
		fieldNames.push_back(expectConsume(TOKEN_IDENTIFIER, "").value);
		expectConsume(TOKEN_COLON, "Expected colon after name");
		fieldExprs.push_back(parseExpression());

		if (tokens[index].type == TOKEN_COMMA)
			expectConsume(TOKEN_COMMA, "Expected comma after field");
	}

	expectConsume(TOKEN_RIGHT_BRACE, "Expected left square bracket");

	return new StructLiteral(moduleName, name, fieldNames, fieldExprs);
}

ASTNode *FileParser::parseSpecial()
{
	auto cur = tokens[index];
	index++;

	expectConsume(TOKEN_LEFT_PAREN, "Expected opening paren");

	if (cur.value == "cast")
	{
		auto type = parseType();
		expectConsume(TOKEN_COMMA, "Expected comma");
		auto expr = parseExpression();

		expectConsume(TOKEN_RIGHT_PAREN, "Expected closing paren");
		return new Cast(type, expr);
	}

	return nullptr;
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
		// std::string tokString = Lexer::tokenEnumToString[p.type];
		std::string tokString = p.value;

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
	for (auto file : files)
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

void Parser::parse(std::filesystem::path p)
{
	// std::cout << "Beginning to parse: " << p << "\n";
	Lexer lex(p);
	lex.display(lex.tokens());

	FileParser fileParser(lex.tokens(), p, this);
	auto ast = fileParser.parse();

	// std::cout << "AST for file: " << p << "\n";
	//	for (ASTNode *node: ast) {
	//		node->print(0);
	//	}
	//	std::cout << std::endl;

	FileInfo file = {p, ast, fileParser.functionSymbols, fileParser.structSymbols};
	files.push_back(file);
}
