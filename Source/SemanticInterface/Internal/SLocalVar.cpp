#include "SLocalVar.h"

#include "SCommon.h"

#include "SMethod.h"
#include "SSyntaxNode.h"
#include "SStatements.h"
#include "SExpression.h"
#include "SClass.h"
#include "SConstructObject.h"

void TSLocalVar::Build(SemanticApi::TGlobalBuildContext build_context)
{
	type.LinkSignature(build_context);
	type.LinkBody(build_context);

	std::vector<SemanticApi::ISMethod*> methods;
	if (GetOwner()->GetMethods(methods, GetSyntax()->GetName()))
		GetSyntax()->Error("Метод не может быть именем переменной!");
	if (GetOwner()->GetClass(GetSyntax()->GetName()) != nullptr)
		GetSyntax()->Error("Класс не может быть именем переменной!");
	SemanticApi::IVariable* t = GetParentStatements()->GetVar(GetSyntax()->GetName(), GetSyntax()->GetStmtId());
	if (t != nullptr&&t != this)
	{
		switch (t->GetVariableType())
		{
		case SemanticApi::VariableType::ClassField:
			GetSyntax()->Error("Локальная переменная перекрывает член класса!");
		case SemanticApi::VariableType::Parameter:
			GetSyntax()->Error("Локальная переменная перекрывает параметр!");
		case SemanticApi::VariableType::Local:
			GetSyntax()->Error("Локальная переменная с таким именем уже существует!");
		default:assert(false);
		}
	}

	construct_object.reset(new TSConstructObject(GetOwner(), GetMethod(), GetParentStatements(), type.GetClass()));

	construct_object->Build(GetSyntax(),GetSyntax()->GetParams(), build_context);

	if (GetSyntax()->IsStatic())
	{
		build_context.static_variables->push_back(this);
	}
	GetParentStatements()->AddVar(this, GetSyntax()->GetStmtId());

	if (GetSyntax()->GetAssignExpr() != nullptr)
	{
		assign_expr.reset(new TSExpression(GetOwner(), GetMethod(), GetParentStatements(), GetSyntax()->GetAssignExpr()));
		assign_expr->Build(build_context);
	}
}

const SemanticApi::ISType * TSLocalVar::GetType() const
{
	return &type;
}

SemanticApi::ISConstructObject * TSLocalVar::GetConstructObject() const
{
	return construct_object.get();
}

SemanticApi::ISOperations::ISExpression * TSLocalVar::GetAssignExpression() const
{
	return assign_expr.get();
}

void TSLocalVar::Accept(SemanticApi::ISStatementVisitor * visitor)
{
	visitor->Visit(this);
}

TSLocalVar::TSLocalVar(TSClass* use_owner, TSMethod* use_method, TSStatements* use_parent, SyntaxApi::ILocalVar* use_syntax)
	:TSStatement(SyntaxApi::TStatementType::VarDecl, use_owner, use_method, use_parent, use_syntax)
	, TVariable(SemanticApi::VariableType::Local)
	, type(use_owner, use_syntax->GetVarType())
{

}

Lexer::TNameId TSLocalVar::GetName()const
{
	return GetSyntax()->GetName();
}

SyntaxApi::ILocalVar* TSLocalVar::GetSyntax()const
{
	return dynamic_cast<SyntaxApi::ILocalVar*>(TSyntaxNode::GetSyntax());
}

TSClass* TSLocalVar::GetClass()const
{
	return dynamic_cast<TSClass*>(type.GetClass());
}

bool TSLocalVar::IsStatic()const
{
	return GetSyntax()->IsStatic();
}
