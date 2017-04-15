#pragma once

#include "../SyntaxTree/SyntaxTreeApi.h"

#include "Statements.h"
#include "Expression.h"

namespace SyntaxInternal
{
	class TIf :public TStatement, public virtual SyntaxApi::IIf
	{
		std::unique_ptr<TExpression> bool_expr;
		std::unique_ptr<TStatements> statements, else_statements;
	public:
		TExpression* GetBoolExpr()const;
		TStatements* GetStatements()const;
		TStatements* GetElseStatements()const;
		TIf(TClass* use_owner, TMethod* use_method, TStatements* use_parent, int use_stmt_id);
		void AnalyzeSyntax(Lexer::ILexer* source);
		void Accept(SyntaxApi::IStatementVisitor* visitor);
	};
}