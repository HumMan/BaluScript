﻿#include "SLocalVar.h"

#include "../Syntax/Expression.h"
#include "../Syntax/Statements.h"
#include "../Syntax/LocalVar.h"
#include "../Syntax/Method.h"

#include "../semanticAnalyzer.h"

#include "SMethod.h"
#include "SSyntaxNode.h"
#include "SStatements.h"
#include "FormalParam.h"
#include "SExpression.h"
#include "SClass.h"
#include "SConstructObject.h"

void TSLocalVar::Build(TGlobalBuildContext build_context)
{
	type.LinkSignature(build_context);
	type.LinkBody(build_context);

	std::vector<TSMethod*> methods;
	if (owner->GetMethods(methods, GetSyntax()->name))
		GetSyntax()->Error("Метод не может быть именем переменной!");
	if (owner->GetClass(GetSyntax()->name) != NULL)
		GetSyntax()->Error("Класс не может быть именем переменной!");
	TVariable* t = parent->GetVar(GetSyntax()->name, GetSyntax()->stmt_id);
	if (t != NULL&&t != this)
	{
		switch (t->GetType())
		{
		case TVariableType::ClassField:
			GetSyntax()->Error("Локальная переменная перекрывает член класса!");
		case TVariableType::Parameter:
			GetSyntax()->Error("Локальная переменная перекрывает параметр!");
		case TVariableType::Local:
			GetSyntax()->Error("Локальная переменная с таким именем уже существует!");
		default:assert(false);
		}
	}

	construct_object.reset(new TSConstructObject(owner, method, parent, type.GetClass()));

	construct_object->Build(GetSyntax(),GetSyntax()->params, build_context);

	if (GetSyntax()->is_static)
	{
		build_context.static_variables->push_back(this);
	}
	parent->AddVar(this, GetSyntax()->stmt_id);

	if (GetSyntax()->assign_expr != NULL)
	{
		assign_expr.reset(new TSExpression(owner, method, parent, GetSyntax()->assign_expr.get()));
		assign_expr->Build(build_context);
	}
}

TSLocalVar::TSLocalVar(TSClass* use_owner, TSMethod* use_method, TSStatements* use_parent, TLocalVar* use_syntax)
	:TSStatement(TStatementType::VarDecl, use_owner, use_method, use_parent, use_syntax)
	, TVariable(TVariableType::Local)
	, type(use_owner, use_syntax->GetVarType())
{

}

TNameId TSLocalVar::GetName()
{
	return GetSyntax()->GetName();
}

TSClass* TSLocalVar::GetClass()
{
	return type.GetClass();
}

bool TSLocalVar::IsStatic()
{
	return GetSyntax()->IsStatic();
}

void TSLocalVar::Run(TStatementRunContext run_context)
{
	if (!IsStatic())
		run_context.local_variables->push_back(TStackValue(false, type.GetClass()));

	//TODO т.к. в GetClassMember создаются временные объекты
	//if (!IsStatic())
	//	assert(GetOffset() == local_variables.size() - 1);//иначе ошибка Build локальных переменных

	if (IsStatic() && (*run_context.static_fields)[GetOffset()].IsInitialized())
	{
		return;
	}
	TStackValue var_object(true, GetClass());
	if (IsStatic())
		var_object.SetAsReference((*run_context.static_fields)[GetOffset()].get());
	else
		var_object.SetAsReference(run_context.local_variables->back().get());

	construct_object->Construct(var_object, run_context);

	if (IsStatic())
	{
		(*run_context.static_fields)[GetOffset()].Initialize();
	}

	if (assign_expr)
		assign_expr->Run(run_context);
}

void TSLocalVar::Destruct(std::vector<TStaticValue> &static_fields, std::vector<TStackValue>& local_variables)
{
	if (!IsStatic())
	{
		TStackValue without_result, var_object(true, GetClass());
		var_object.SetAsReference(local_variables.back().get());
		construct_object->Destruct(var_object, TGlobalRunContext(&static_fields));
	}
}