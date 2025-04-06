#include "generator.h"
#include "parser.h"

Generator::Generator(Parser *parser): parser(parser), builder(ctx), module("main", ctx) {}

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
		currentFile = &fileInfo;
		for (auto node: fileInfo.nodes)
		{
			node->codegen(this);
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

llvm::Value* FunctionDefinition::codegen(Generator *gen)
{
	if (!body) return nullptr;

	llvm::Function *func = gen->functionSymbols[moduleName][name];
	llvm::BasicBlock *entry = llvm::BasicBlock::Create(gen->module.getContext(), "entry", func);	
	gen->builder.SetInsertPoint(entry);

	for (auto node : body.value())
	{
		node->codegen(gen);
	}

	if (func->getReturnType()->isVoidTy())
		gen->builder.CreateRetVoid();

	return func;
}

llvm::Value* Return::codegen(Generator *gen)
{
	return gen->builder.CreateRet(gen->builder.getInt32(value));
}

llvm::Value* FunctionCall::codegen(Generator *gen)
{
	if (!gen->currentFile->functionIncluded(name))
	{
		std::cerr << "function does not exist: " << name;
		exit(1);
	}

	// gen->displayFunctionSymbols();
	llvm::Function *func = gen->functionSymbols[moduleName][name];
	std::vector<llvm::Value *> callArgs;

	for (auto arg : params)
	{
		auto s = static_cast<StringLiteral *>(arg);
		callArgs.push_back(gen->builder.CreateGlobalStringPtr(s->value));
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
