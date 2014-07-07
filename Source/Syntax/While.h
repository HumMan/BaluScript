#pragma once

#include "Statement.h"

class TExpression;
class TFor;

class TWhile:public TStatement
{
	std::shared_ptr<TExpression> compare;
	std::shared_ptr<TStatements> statements;
	void operator=(const TFor& use_source);
public:
	TWhile(TClass* use_owner, TMethod* use_method, TStatements* use_parent, int use_stmt_id);
	void AnalyzeSyntax(TLexer& source);
	void Accept(TStatementVisitor* visitor);
};