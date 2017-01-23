#pragma once

#include <memory>

#include "Statement.h"

class TStatements;
class TClass;
class TExpression;

class TFor :public TStatement
{
	std::unique_ptr<TExpression> compare;
	std::unique_ptr<TStatements> increment;
	std::unique_ptr<TStatements> statements;
public:
	TExpression* GetCompare()const
	{
		return compare.get();
	}
	TStatements* GetIncrement()const
	{
		return increment.get();
	}
	TStatements* GetStatements()const
	{
		return statements.get();
	}
	TFor(TClass* use_owner, TMethod* use_method, TStatements* use_parent, int use_stmt_id);
	void AnalyzeSyntax(Lexer::ILexer* source);
	void Accept(TStatementVisitor* visitor);
};