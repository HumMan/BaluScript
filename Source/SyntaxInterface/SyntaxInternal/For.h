#pragma once

#include "../SyntaxTreeApi.h"

#include "Statements.h"
#include "Expression.h"

namespace SyntaxInternal
{
	class TClass;

	class TFor :public TStatement, public SyntaxApi::IFor
	{
		std::unique_ptr<TExpression> compare;
		std::unique_ptr<TStatements> increment;
		std::unique_ptr<TStatements> statements;
	public:
		TExpression* GetCompare()const;
		TStatements* GetIncrement()const;
		TStatements* GetStatements()const;
		TFor(TClass* use_owner, TMethod* use_method, TStatements* use_parent, int use_stmt_id);
		void AnalyzeSyntax(Lexer::ILexer* source);
		void Accept(SyntaxApi::IStatementVisitor* visitor);
	};
}