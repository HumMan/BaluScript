#pragma once

#include "Statement.h"

class TExpression;
class TStatements;

class TIf:public TStatement
{
	std::shared_ptr<TExpression> bool_expr;
	std::shared_ptr<TStatements> statements, else_statements;
public:
	TIf(TClass* use_owner, TMethod* use_method, TStatements* use_parent, int use_stmt_id);
	void AnalyzeSyntax(TLexer& source);
};