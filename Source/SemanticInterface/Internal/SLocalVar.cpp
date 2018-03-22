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
	bool is_using_var = GetSyntax()->GetVarType() == nullptr;//используется ключевое слово var
	if (!is_using_var)
	{
		type.LinkSignature(build_context);
		type.LinkBody(build_context);
	}

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

	if (is_using_var)
	{

		auto right = new TSExpression(GetOwner(), GetMethod(), GetParentStatements(), GetSyntax()->GetAssignExpr());
		right->Build(build_context);

		//обновляем тип переменной исходя из результат выполнения выражения
		TSClass* assign_result = dynamic_cast<TSClass *>(right->GetFormalParameter().GetClass());
		type = TSType(GetOwner(), assign_result);
		construct_object.reset(new TSConstructObject(GetOwner(), GetMethod(), GetParentStatements(), type.GetClass()));

		construct_object->Build(GetSyntax(), GetSyntax()->GetParams(), build_context);
		GetParentStatements()->AddVar(this, GetSyntax()->GetStmtId());

		//т.к. для присваивания значения у нас нет конструкции - создаём её
		TSOperation *left = new TSExpression::TGetLocal(this);

		auto bin_operator = FindBinaryOperator(left, right, right->GetSyntax(), GetOwner(), Lexer::TOperator::Assign);
		
		TSExpression_TMethodCall* method_call = nullptr;

		method_call = new TSExpression_TMethodCall(SemanticApi::TMethodCallType::Operator);
		
		std::vector<TSOperation*> param_expressions;
		param_expressions.push_back(left);
		param_expressions.push_back(right);

		method_call->Build(param_expressions, dynamic_cast<TSMethod*>(bin_operator));

		assign_expr.reset(new TSExpression(GetOwner(), GetMethod(), GetParentStatements(), GetSyntax()->GetAssignExpr(), method_call));
	}
	else
	{
		construct_object.reset(new TSConstructObject(GetOwner(), GetMethod(), GetParentStatements(), type.GetClass()));

		construct_object->Build(GetSyntax(), GetSyntax()->GetParams(), build_context);

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
