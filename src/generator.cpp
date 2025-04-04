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
		for (auto node: fileInfo.nodes)
		{
			if (dynamic_cast<FunctionDefinition*>(node) != nullptr)
			{
				auto func = static_cast<FunctionDefinition *>(node);

				llvm::Function *f= module.getFunction(func->name);
				if (f != nullptr) continue;
	
				std::vector<llvm::Type *> paramTypes;

				for (auto type: func->paramTypes)
				{
					paramTypes.push_back(typeInfo(type).type(ctx));
				}

				llvm::Type *returnType = typeInfo(func->returnType).type(ctx);
				llvm::FunctionType *funcType = llvm::FunctionType::get(returnType, paramTypes, false);

				llvm::Function::Create(funcType, llvm::Function::ExternalLinkage, func->name, module);
			}
		}
	}
}

void Generator::generate()
{
	generateFunctionDefinitions();

	for (auto fileInfo: parser->files)
	{
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

	llvm::Function *func = gen->module.getFunction(name);
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
	llvm::Function *func = gen->module.getFunction(name);
	std::vector<llvm::Value *> callArgs;

	for (auto arg : params)
	{
		auto s = static_cast<StringLiteral *>(arg);
		callArgs.push_back(gen->builder.CreateGlobalStringPtr(s->value));
	}

	return gen->builder.CreateCall(func, callArgs);
}
