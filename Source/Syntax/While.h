#pragma once

#include <memory>

#include "Statement.h"

class TExpression;
class TFor;

class TWhile:public TStatement
{
	friend class TSWhile;
	std::unique_ptr<TExpression> compare;
	std::unique_ptr<TStatements> statements;
	void operator=(const TFor& use_source);
public:
	TWhile(TClass* use_owner, TMethod* use_method, TStatements* use_parent, int use_stmt_id);
	void AnalyzeSyntax(Lexer::ILexer* source);
	void Accept(TStatementVisitor* visitor);
};