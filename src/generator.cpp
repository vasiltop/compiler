#include "generator.h"
#include "parser.h"

Generator::Generator(Parser *parser): parser(parser), builder(ctx), module("main", ctx) {}

GScope::GScope(GScope *parent): parent(parent) {}

std::pair<llvm::Value *, GType> GScope::getVar(std::string name)
{
	GScope *cur = this;
	unsigned level = 0;

	while (cur)
	{
		if (cur->variables.find(name) != cur->variables.end())
		{
			return cur->variables[name];	
		}
		else
		{
			level++;
			cur = cur->parent;
		}
	}

	return std::pair{nullptr, GType{nullptr, 0}};
}

llvm::Type *GType::type(llvm::LLVMContext &ctx)
{
	if (depth > 0)
	{
		return llvm::PointerType::get(ctx, 0);
	}

	return elementType;
}

GType Generator::typeInfo(Type *type)
{
	GType gType;
	llvm::Type *ty = nullptr;

	if (type->name == "i32")
	{
		ty = llvm::Type::getInt32Ty(ctx);
	}
	else if (type->name == "void")
	{
	ty = llvm::Type::getVoidTy(ctx);
	}
	else if (type->name == "u8")
	{
		ty = llvm::Type::getInt8Ty(ctx);
	}
	
	gType.elementType = ty;
	gType.depth = type->pointerLevel;

	return gType;
}

void Generator::generateFunctionDefinitions()
{
	for (auto fileInfo: parser->files)
	{
		std::string moduleName = parser->pathToModule[fileInfo.path];

		for (auto node: fileInfo.nodes)
		{
			if (dynamic_cast<FunctionDefinition*>(node) != nullptr)
			{
				auto func = static_cast<FunctionDefinition *>(node);
				std::vector<llvm::Type *> paramTypes;

				for (auto type: func->paramTypes)
				{
					paramTypes.push_back(typeInfo(type).type(ctx));
				}

				llvm::Type *returnType = typeInfo(func->returnType).type(ctx);
				llvm::FunctionType *funcType = llvm::FunctionType::get(returnType, paramTypes, false);

				functionSymbols[moduleName][func->name] = llvm::Function::Create(funcType, llvm::Function::ExternalLinkage, func->name, module);
			}
		}
	}
}

void Generator::generate()
{
	generateFunctionDefinitions();


	for (auto fileInfo: parser->files)
	{
		GScope *scope = new GScope(nullptr);
		currentFile = &fileInfo;
		for (auto node: fileInfo.nodes)
		{
			node->codegen(scope, this);
		}
	}

	std::string filename = "out.ll";

	std::error_code errorCode;
	llvm::raw_fd_ostream outFile(filename, errorCode);

	if (errorCode)
	{
		llvm::errs() << "Error opening file '" << filename << "': " << errorCode.message() << "\n";
		return;
	}

	module.print(outFile, nullptr);
	outFile.close();

	system("clang out.ll");
}

llvm::Value* FunctionDefinition::codegen(GScope *scope, Generator *gen)
{
	if (!body) return nullptr;

	GScope *funcScope = new GScope(scope);

	llvm::Function *func = gen->functionSymbols[moduleName][name];
	llvm::BasicBlock *entry = llvm::BasicBlock::Create(gen->module.getContext(), "entry", func);	
	gen->builder.SetInsertPoint(entry);

	unsigned i = 0;

	for (auto &arg: func->args())
	{
		llvm::AllocaInst *alloc = gen->builder.CreateAlloca(arg.getType(), nullptr, arg.getName());
		gen->builder.CreateStore(&arg, alloc);

		GType ty = gen->typeInfo(paramTypes[i]);
		funcScope->variables[paramNames[i]] = std::pair{alloc, ty};
		++i;
	}

	for (auto node : body.value())
	{
		node->codegen(funcScope, gen);
	}

	if (func->getReturnType()->isVoidTy())
		gen->builder.CreateRetVoid();

	return func;
}

llvm::Value* StringLiteral::codegen(GScope *scope, Generator *gen)
{
	return gen->builder.CreateGlobalStringPtr(value);	
}

llvm::Value* IntLiteral::codegen(GScope *scope, Generator *gen)
{
	return gen->builder.getInt32(value);
}

llvm::Value* BoolLiteral::codegen(GScope *scope, Generator *gen)
{
	return gen->builder.getInt1(value);
}

llvm::Value* Variable::codegen(GScope *scope, Generator *gen)
{
	auto var = scope->getVar(name);

	if (!var.first) 
	{
		std::cout << "Could not find variable with name: " << name << "\n";
		return nullptr;
	}

	return gen->builder.CreateLoad(var.second.type(gen->ctx), var.first);
}

llvm::Value *BinaryExpr::codegen(GScope *scope, Generator *gen)
{
	llvm::Value *lhsValue = lhs->codegen(scope, gen);
	llvm::Value *rhsValue = rhs->codegen(scope, gen);

	if (!lhsValue || !rhsValue) return nullptr;

	switch (op.type)
	{
		case TOKEN_OPERATOR_PLUS:
        return gen->builder.CreateAdd(lhsValue, rhsValue);
    case TOKEN_OPERATOR_MINUS:
        return gen->builder.CreateSub(lhsValue, rhsValue);
    case TOKEN_OPERATOR_MUL:
        return gen->builder.CreateMul(lhsValue, rhsValue);
    case TOKEN_OPERATOR_DIV:
        return gen->builder.CreateSDiv(lhsValue, rhsValue);
    case TOKEN_OPERATOR_EQUAL:
        return gen->builder.CreateICmpEQ(lhsValue, rhsValue);
    case TOKEN_OPERATOR_NOT_EQUAL:
        return gen->builder.CreateICmpNE(lhsValue, rhsValue);
    case TOKEN_OPERATOR_LESS:
        return gen->builder.CreateICmpSLT(lhsValue, rhsValue);
    case TOKEN_OPERATOR_GREATER:
        return gen->builder.CreateICmpSGT(lhsValue, rhsValue);
    case TOKEN_OPERATOR_LESS_EQUAL:
        return gen->builder.CreateICmpSLE(lhsValue, rhsValue);
    case TOKEN_OPERATOR_GREATER_EQUAL:
        return gen->builder.CreateICmpSGE(lhsValue, rhsValue);
    case TOKEN_OPERATOR_AND:
        return gen->builder.CreateAnd(lhsValue, rhsValue);
    case TOKEN_OPERATOR_OR:
        return gen->builder.CreateOr(lhsValue, rhsValue);
    default:
        return nullptr;
	}
}

llvm::Value* UnaryExpr::codegen(GScope *scope, Generator *gen)
{
	auto* val = expr->codegen(scope, gen);

	if (!val) return nullptr;

	switch (op.type)
	{
		case TOKEN_OPERATOR_MINUS:
			return gen->builder.CreateNeg(val);
		case TOKEN_OPERATOR_NOT:
			return gen->builder.CreateNot(val);
		case TOKEN_POINTER:
			if (!val->getType()->isPointerTy()) return nullptr;
			//return builder.CreateLoad(val)
		default:
			return nullptr;

	}
}

llvm::Value* VariableDecl::codegen(GScope *scope, Generator *gen)
{
	auto ty = gen->typeInfo(type);
	auto val = expr->codegen(scope, gen);

	auto *alloc = gen->builder.CreateAlloca(ty.type(gen->ctx), nullptr, varName);
	gen->builder.CreateStore(val, alloc);
	scope->variables[varName] = std::pair{alloc, ty};

	return alloc;
}

llvm::Value* Return::codegen(GScope *scope, Generator *gen)
{
	auto e = expr->codegen(scope, gen);

	if (!e)
	{
		std::cout << "Invalid expression in return\n";
		return nullptr;
	}

	return gen->builder.CreateRet(e);
}

llvm::Value* FunctionCall::codegen(GScope *scope, Generator *gen)
{
	if (!gen->functionSymbols.count(moduleName))
	{
		std::cerr << "module does not exist: " << moduleName << "\n";
		exit(1);
	}

	if (!gen->functionSymbols[moduleName].count(name))
	{
		std::cerr << "function does not exist: " << name << "\n";
		exit(1);
	}

	// gen->displayFunctionSymbols();
	llvm::Function *func = gen->functionSymbols[moduleName][name];
	std::vector<llvm::Value *> callArgs;

	for (auto arg : params)
	{
		callArgs.push_back(arg->codegen(scope, gen));
	}

	return gen->builder.CreateCall(func, callArgs);
}

void Generator::displayFunctionSymbols()
{
	 std::cout << "Function Symbols Map Contents:\n";
	 std::cout << "=============================\n";

	 // Iterate through outer map (module names)
	 for (const auto& modulePair : functionSymbols) {
		 const std::string& moduleName = modulePair.first;
		 std::cout << "Module: " << moduleName << "\n";

		 // Iterate through inner map (function names to Function*)
		 for (const auto& functionPair : modulePair.second) {
			 const std::string& functionName = functionPair.first;
			 llvm::Function* func = functionPair.second;

			 std::cout << "  Function: " << functionName 
				 << " (" << (void*)func << ")";

			 // If you want to print more details about the function
			 if (func) {
				 std::cout << " - " << func->getName().str()
					 << ", args: " << func->arg_size();
			 }
			 std::cout << "\n";
		 }
	 }
	 std::cout << "=============================\n";
}
