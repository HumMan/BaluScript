#include "Statement.h"

#include "Statements.h"
#include "ClassField.h"

#include "Method.h"
#include "Class.h"

using namespace SyntaxInternal;

void TStatement::SetStmtId(int use_id)
{
	stmt_id = use_id;
}
TStatement::TStatement(SyntaxApi::TStatementType use_stmt_type, TClass* use_owner, TMethod* use_method, TStatements* use_parent, int use_stmt_id) :
method(use_method), parent(use_parent), owner(use_owner),
stmt_id(use_stmt_id), stmt_type(use_stmt_type)
{
}

SyntaxApi::TStatementType TStatement::GetType()const
{
	return stmt_type;
}

int TStatement::GetStmtId()const
{
	return stmt_id;
}
SyntaxApi::IMethod* TStatement::GetMethod()const
{
	return method;
}
TMethod* TStatement::GetMethodT()const
{
	return method;
}
SyntaxApi::IStatements* TStatement::GetParent()const
{
	return parent;
}
TStatements* TStatement::GetParentT()const
{
	return parent;
}
SyntaxApi::IClass* TStatement::GetOwner()const
{
	return owner;
}
TClass* TStatement::GetOwnerT()const
{
	return owner;
}
TStatement::~TStatement()
{
}