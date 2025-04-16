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
	gType.depth = type->pointerLevel;

	if (auto ar = dynamic_cast<ArrayType *>(type))
	{
		auto elemType = typeInfo(ar->type);
		auto arrayType = llvm::ArrayType::get(elemType.type(ctx), ar->size);
		gType.elementType = arrayType;

		return gType;
	}

	if (auto st = dynamic_cast<StructType *>(type))
	{
		gType.elementType = structSymbols[st->moduleName][st->name].type;
		return gType;
	}

	if (type->name == "i64" || type->name == "u64")
	{
		ty = llvm::Type::getInt64Ty(ctx);
	}
	else if (type->name == "i32" || type->name == "u32")
	{
		ty = llvm::Type::getInt32Ty(ctx);
	}
	else if (type->name == "i16" || type->name == "u16")
	{
		ty = llvm::Type::getInt16Ty(ctx);
	}
	else if (type->name == "i8" || type->name == "u8")
	{
		ty = llvm::Type::getInt8Ty(ctx);
	}
	else if (type->name == "f64") {
		ty = llvm::Type::getDoubleTy(ctx);
	}
	else if (type->name == "f32") {
		ty = llvm::Type::getFloatTy(ctx);
	}
	else if (type->name == "bool")
	{
		ty = llvm::Type::getInt1Ty(ctx);
	}
	else if (type->name == "void")
	{
		ty = llvm::Type::getVoidTy(ctx);
	}
	else if (type->name == "string")
	{
		ty = llvm::Type::getInt8Ty(ctx);
		gType.depth = 1;
	}
	else if (type->name == "char") {
    ty = llvm::Type::getInt8Ty(ctx);
	}
	
	gType.elementType = ty;

	return gType;
}

void Generator::generateDefinitions()
{
	for (auto fileInfo: parser->files)
	{
		std::string moduleName = parser->pathToModule[fileInfo.path];

		for (auto node: fileInfo.nodes)
		{
			if (auto func = dynamic_cast<FunctionDefinition*>(node))
			{
				std::vector<llvm::Type *> paramTypes;

				for (auto type: func->paramTypes)
				{
					paramTypes.push_back(typeInfo(type).type(ctx));
				}

				llvm::Type *returnType = typeInfo(func->returnType).type(ctx);
				llvm::FunctionType *funcType = llvm::FunctionType::get(returnType, paramTypes, false);

				functionSymbols[moduleName][func->name] = llvm::Function::Create(funcType, llvm::Function::ExternalLinkage, func->name, module);
			}
			else if (auto structDef = dynamic_cast<StructDefinition *>(node))
			{
				std::vector<llvm::Type *> memberTypes;

				for (auto &type: structDef->fieldTypes)
				{
					auto ty = typeInfo(type);
					memberTypes.push_back(ty.type(ctx));
				}

				std::string name = structDef->moduleName + ":" + structDef->name;
				structSymbols[moduleName][structDef->name] = {llvm::StructType::create(ctx, memberTypes, name), structDef->fieldNames};
			}
		}
	}
}

void Generator::generate()
{
	generateDefinitions();


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

	body->codegen(funcScope, gen);

	if (func->getReturnType()->isVoidTy())
		gen->builder.CreateRetVoid();

	return func;
}


llvm::Value* Block::codegen(GScope *scope, Generator *gen)
{
	GScope *blockScope = new GScope(scope);

	for (auto &node: body)
	{
		node->codegen(blockScope, gen);
	}
	
	return nullptr;
}

llvm::Value* StringLiteral::codegen(GScope *scope, Generator *gen)
{
	return gen->builder.CreateGlobalStringPtr(value);	
}

llvm::Value* CharLiteral::codegen(GScope *scope, Generator *gen)
{
	return gen->builder.getInt8(value);
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

	if (gen->inReferenceContext)
		return var.first;

	return gen->builder.CreateLoad(var.second.type(gen->ctx), var.first);
}

llvm::Value *BinaryExpr::codegen(GScope *scope, Generator *gen)
{
	llvm::Value *lhsValue = lhs->codegen(scope, gen);
	llvm::Value *rhsValue = rhs->codegen(scope, gen);

	if (!lhsValue || !rhsValue) return nullptr;

	auto lhsType = gen->expressionType(lhs, scope);
	auto rhsType = gen->expressionType(rhs, scope);

	switch (op.type)
	{
		case TOKEN_OPERATOR_PLUS:
			{
				if (lhsType.isPointer() && rhsType.type(gen->ctx)->isIntegerTy()) {
					return gen->builder.CreateGEP(
							lhsType.elementType,
							lhsValue, 
							rhsValue, 
							"ptr_add");
				}
				if (rhsType.isPointer() && lhsType.type(gen->ctx)->isIntegerTy()) {
					return gen->builder.CreateGEP(
							rhsType.elementType,
							rhsValue, 
							lhsValue, 
							"ptr_add");
				}
				return gen->builder.CreateAdd(lhsValue, rhsValue);
			}
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

GType Generator::expressionType(ASTNode *expr, GScope *scope)
{
	if (auto* intLit = dynamic_cast<IntLiteral*>(expr)) {
		return GType{llvm::Type::getInt32Ty(ctx), 0};
	}

	if (auto* stringLit = dynamic_cast<StringLiteral*>(expr)) {
		return GType{llvm::Type::getInt8Ty(ctx), 1};
	}

	if (auto* var = dynamic_cast<Variable*>(expr)) {
		return scope->getVar(var->name).second;
	}

	if (auto* unary = dynamic_cast<UnaryExpr*>(expr)) {
		GType subType = expressionType(unary->expr, scope);
		if (unary->op.type == TOKEN_POINTER) {
			return GType{subType.elementType, subType.depth - 1};
		}
		if (unary->op.type == TOKEN_REFERENCE) {
			return GType{subType.elementType, subType.depth + 1};
		}
		return subType;
	}

	if (auto* binary = dynamic_cast<BinaryExpr*>(expr)) {
		GType lhsType = expressionType(binary->lhs, scope);
		GType rhsType = expressionType(binary->rhs, scope);
		return lhsType; // TODO: Fix this
	}

	return GType{llvm::Type::getInt32Ty(ctx), 0};
}

llvm::Value* Cast::codegen(GScope *scope, Generator *gen)
{
	auto val = expr->codegen(scope, gen);

	GType targetGType = gen->typeInfo(type);
	GType sourceGType = gen->expressionType(expr, scope);

	auto targetType = targetGType.type(gen->ctx);
	auto sourceType = sourceGType.type(gen->ctx);

	if (sourceType->isIntegerTy() && targetType->isIntegerTy()) {
		unsigned srcBits = sourceType->getIntegerBitWidth();
		unsigned dstBits = targetType->getIntegerBitWidth();

		if (srcBits == dstBits) {
			return val;
		}
		else if (srcBits < dstBits) {
			if (type->isSigned()) {
				return gen->builder.CreateSExt(val, targetType, "sext");
			} else {
				return gen->builder.CreateZExt(val, targetType, "zext");
			}
		} else {
			return gen->builder.CreateTrunc(val, targetType, "trunc");
		}
	}

	return nullptr;
}


llvm::Value* While::codegen(GScope *scope, Generator *gen)
{
	auto func = gen->builder.GetInsertBlock()->getParent();

	GScope* whileScope = new GScope(scope);

	auto condBlock = llvm::BasicBlock::Create(gen->ctx, "cond", func);
	auto bodyBlock = llvm::BasicBlock::Create(gen->ctx, "body", func);
	auto mergeBlock = llvm::BasicBlock::Create(gen->ctx, "merge", func);

	gen->builder.CreateBr(condBlock);
	gen->builder.SetInsertPoint(condBlock);

	auto cond = condition->codegen(whileScope, gen);
	gen->builder.CreateCondBr(cond, bodyBlock, mergeBlock);

	gen->builder.SetInsertPoint(bodyBlock);

	body->codegen(whileScope, gen);
	gen->builder.CreateBr(condBlock);

	gen->builder.SetInsertPoint(mergeBlock);

	return nullptr;
}

llvm::Value* Conditional::codegen(GScope *scope, Generator *gen)
{

	auto func = gen->builder.GetInsertBlock()->getParent();
	auto mergeBB = llvm::BasicBlock::Create(gen->ctx, "if.end", func);

	for (auto &condition: conditions)
	{
		auto* condBB = llvm::BasicBlock::Create(gen->ctx, "if.cond", func);
		auto* thenBB = llvm::BasicBlock::Create(gen->ctx, "if.then", func);
		auto* elseBB = llvm::BasicBlock::Create(gen->ctx, "if.else", func);
		gen->builder.CreateBr(condBB);
    gen->builder.SetInsertPoint(condBB);

		if (condition.first)
		{
			GScope* condScope = new GScope(scope);
      auto* condValue = condition.first->codegen(condScope, gen);
      if (!condValue) return nullptr;

			gen->builder.CreateCondBr(condValue, thenBB, elseBB);
		}
		else
		{
			gen->builder.CreateBr(thenBB);
		}

		gen->builder.SetInsertPoint(thenBB);

		{
			GScope* thenScope = new GScope(scope);
			condition.second->codegen(thenScope, gen);
			gen->builder.CreateBr(mergeBB);
		}

		gen->builder.SetInsertPoint(elseBB);

	}

	gen->builder.CreateBr(mergeBB);
	gen->builder.SetInsertPoint(mergeBB);

	return nullptr;
}

llvm::Value* Assign::codegen(GScope *scope, Generator *gen)
{
	gen->inReferenceContext = true;
	auto lvalue = lhs->codegen(scope, gen);
	gen->inReferenceContext = false;

	auto rvalue = rhs->codegen(scope, gen);
	return gen->builder.CreateStore(rvalue, lvalue);
}

llvm::Value* VariableAccess::codegen(GScope *scope, Generator *gen)
{
	auto var = scope->getVar(varName);

	for (auto& index: indexes)
	{
		if (auto arrayIndex = dynamic_cast<ArrayIndex *>(index))
		{
			auto indexValue = arrayIndex->expr->codegen(scope, gen);

			auto ptr = gen->builder.CreateGEP(var.second.elementType, var.first, {gen->builder.getInt32(0), indexValue});

			if (var.second.elementType->isArrayTy())
			{
				auto newType = var.second.elementType->getArrayElementType();
				var.first = ptr;
				var.second.elementType = newType;
			}
		}
		else if (auto structField = dynamic_cast<StructField *>(index))
		{
			llvm::StructType* structType = llvm::cast<llvm::StructType>(var.second.elementType);

			auto fullName = structType->getName();
			size_t colonPos = fullName.find(':');
			auto module = fullName.substr(0, colonPos);
      auto name = fullName.substr(colonPos + 1);

			StructInfo info = gen->structSymbols[module.str()][name.str()];
			unsigned int fieldIndex = info.getFieldIndex(structField->fieldName);

			var.first = gen->builder.CreateStructGEP(
					structType,
					var.first,
					fieldIndex,
					varName + "." + structField->fieldName
					);

			var.second.elementType = structType->getElementType(fieldIndex);
		}
	}
	
	if (gen->inReferenceContext)
	{
		return var.first;
	}

	return gen->builder.CreateLoad(var.second.elementType, var.first);
}

llvm::Value* UnaryExpr::codegen(GScope *scope, Generator *gen)
{
	switch (op.type)
	{
		case TOKEN_OPERATOR_MINUS:
			{
				auto val = expr->codegen(scope, gen);
				if (!val) return nullptr;
				return gen->builder.CreateNeg(val);
			}
		case TOKEN_OPERATOR_NOT:
			{
			auto val = expr->codegen(scope, gen);
			if (!val) return nullptr;
			return gen->builder.CreateNot(val);
			}
		case TOKEN_POINTER:
			{
				auto val = expr->codegen(scope, gen);
				auto ty = gen->expressionType(expr, scope);

				if (gen->inReferenceContext)
				{
					if (dynamic_cast<Variable *>(expr))
					{
						return gen->builder.CreateLoad(ty.type(gen->ctx)->getPointerTo(), val);
					}
				}

				GType loadedType{ty.elementType, ty.depth - 1};
				return gen->builder.CreateLoad(loadedType.type(gen->ctx), val);
			}
		case TOKEN_REFERENCE:
			{
				gen->inReferenceContext = true;
				auto val = expr->codegen(scope, gen);
				if (!val) return nullptr;
				gen->inReferenceContext = false;

				if (!dynamic_cast<Variable *>(expr))
				{
					auto ty = gen->expressionType(expr, scope);
					auto alloc = gen->builder.CreateAlloca(ty.type(gen->ctx)->getPointerTo());
					gen->builder.CreateStore(val, alloc);
					return alloc;
				}

				return val;

			}
		default:
			return nullptr;
	}
}

llvm::Value* ArrayLiteral::codegen(GScope *scope, Generator *gen)
{
	std::vector<llvm::Value *> elements;

	for (auto val: values)
	{
		elements.push_back(val->codegen(scope, gen));
	}

	auto type = llvm::ArrayType::get(elements[0]->getType(), elements.size());
	auto alloc = gen->builder.CreateAlloca(type);

	for (size_t i = 0; i < elements.size(); ++i)
	{
		auto ptr = gen->builder.CreateGEP(type, alloc, {gen->builder.getInt32(0), gen->builder.getInt32(i)});
		gen->builder.CreateStore(elements[i], ptr);
	}

	return gen->builder.CreateLoad(type, alloc);
}

unsigned int StructInfo::getFieldIndex(std::string fieldName)
{
	int fieldIndex = -1;

	for (unsigned i = 0; i < fieldNames.size(); i++) {
		if (fieldNames[i] == fieldName) {
			fieldIndex = i;
			break;
		}
	}

	return fieldIndex;
}

llvm::Value* StructLiteral::codegen(GScope *scope, Generator *gen)
{
	StructInfo info = gen->structSymbols[moduleName][name];
	llvm::Value* alloc = gen->builder.CreateAlloca(info.type);

	for (size_t i = 0; i < fieldNames.size(); ++i) {
		unsigned int fieldIndex = info.getFieldIndex(fieldNames[i]);
		llvm::Value* fieldValue = fieldExprs[i]->codegen(scope, gen);

		llvm::Value* fieldPtr = gen->builder.CreateStructGEP(
				info.type,
				alloc,
				fieldIndex,
				"structfield." + fieldNames[i]
				);

		gen->builder.CreateStore(fieldValue, fieldPtr);
	}
	
	return gen->builder.CreateLoad(info.type, alloc);
}

llvm::Value* VariableDecl::codegen(GScope *scope, Generator *gen)
{
	auto ty = gen->typeInfo(type);
	auto val = expr->codegen(scope, gen);
	//ty.type(gen->ctx)->print(llvm::errs());
	auto alloc = gen->builder.CreateAlloca(ty.type(gen->ctx));

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

	 for (const auto& modulePair : functionSymbols) {
		 const std::string& moduleName = modulePair.first;
		 std::cout << "Module: " << moduleName << "\n";

		 for (const auto& functionPair : modulePair.second) {
			 const std::string& functionName = functionPair.first;
			 llvm::Function* func = functionPair.second;

			 std::cout << "  Function: " << functionName 
				 << " (" << (void*)func << ")";

			 if (func) {
				 std::cout << " - " << func->getName().str()
					 << ", args: " << func->arg_size();
			 }
			 std::cout << "\n";
		 }
	 }
	 std::cout << "=============================\n";
}
