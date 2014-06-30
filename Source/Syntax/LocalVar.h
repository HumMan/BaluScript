#pragma once

#include "Statement.h"
#include "Variable.h"
#include "Type.h"

class TExpression;

class TLocalVar:public TStatement,public TVariable
{
	TType type;
	TNameId name;
	std::shared_ptr<TExpression> assign_expr;
	std::vector<std::shared_ptr<TExpression>> params;
	bool is_static;
public:
	TLocalVar(TClass* use_owner, TMethod* use_method, TStatements* use_parent, int use_stmt_id);
	void AnalyzeSyntax(TLexer& source);
	TNameId GetName();
	bool IsStatic();

};