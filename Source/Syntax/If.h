﻿#pragma once

#include <memory>

#include "Statement.h"

class TExpression;
class TStatements;

class TIf:public TStatement
{
	friend class TSIf;
	std::unique_ptr<TExpression> bool_expr;
	std::unique_ptr<TStatements> statements, else_statements;
public:
	TIf(TClass* use_owner, TMethod* use_method, TStatements* use_parent, int use_stmt_id);
	void AnalyzeSyntax(Lexer::ILexer* source);
	void Accept(TStatementVisitor* visitor);
};