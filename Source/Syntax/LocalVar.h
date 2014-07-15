#pragma once

#include "Statement.h"
#include "Type.h"

class TExpression;

class TLocalVar:public TStatement
{
	friend class TSLocalVar;
	TType type;
	TNameId name;
	std::unique_ptr<TExpression> assign_expr;
	std::vector<std::unique_ptr<TExpression>> params;
	bool is_static;
public:
	TLocalVar(TClass* use_owner, TMethod* use_method, TStatements* use_parent, int use_stmt_id);
	void AnalyzeSyntax(TLexer& source);
	TNameId GetName();
	bool IsStatic();
	TType* GetVarType()
	{
		return &type;
	}
	void Accept(TStatementVisitor* visitor);
};