#include "generator.h"

Generator::Generator(Parser parser): parser(parser), builder(ctx), module("main", ctx) {}

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

void Generator::generate()
{
	// Generate definitions for globals (Struct, Function)
	
	for (auto fileInfo: parser.files)
	{
		for (auto node: fileInfo.nodes)
		{
			if (dynamic_cast<FunctionDefinition*>(node) != nullptr)
			{
				auto func = static_cast<FunctionDefinition *>(node);

				if (functions.count(func->name)) continue;

				std::vector<llvm::Type *> paramTypes;

				for (auto type: func->paramTypes)
				{
					paramTypes.push_back(typeInfo(type).type(ctx));
				}

				llvm::Type *returnType = typeInfo(func->returnType).type(ctx);
				llvm::FunctionType *funcType = llvm::FunctionType::get(returnType, paramTypes, false);
				
				functions[func->name] = llvm::Function::Create(funcType, llvm::Function::ExternalLinkage, func->name, module);
			}
		}
	}

	module.print(llvm::outs(), nullptr);
}
