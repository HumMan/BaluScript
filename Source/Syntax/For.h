#pragma once

#include "Statement.h"

class TStatements;
class TClass;
class TExpression;

class TFor :public TStatement
{
	std::shared_ptr<TExpression> compare;
	std::shared_ptr<TStatements> increment;
	std::shared_ptr<TStatements> statements;
public:
	TFor(TClass* use_owner, TMethod* use_method, TStatements* use_parent, int use_stmt_id);
	void AnalyzeSyntax(TLexer& source);
};