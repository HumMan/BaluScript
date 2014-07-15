#pragma once

#include "Statement.h"

class TStatements;
class TClass;
class TExpression;

class TFor :public TStatement
{
	friend class TSFor;
	std::unique_ptr<TExpression> compare;
	std::unique_ptr<TStatements> increment;
	std::unique_ptr<TStatements> statements;
public:
	TFor(TClass* use_owner, TMethod* use_method, TStatements* use_parent, int use_stmt_id);
	void AnalyzeSyntax(TLexer& source);
	void Accept(TStatementVisitor* visitor);
};