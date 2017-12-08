#pragma once

#include "../SyntaxTreeApi.h"

#include "Expression.h"
#include "Statements.h"


namespace SyntaxInternal
{
	class TWhile :public TStatement, public SyntaxApi::IWhile
	{
		std::unique_ptr<TExpression> compare;
		std::unique_ptr<TStatements> statements;
	public:
		TWhile(TClass* use_owner, TMethod* use_method, TStatements* use_parent, int use_stmt_id);
		void AnalyzeSyntax(Lexer::ILexer* source);
		void Accept(SyntaxApi::IStatementVisitor* visitor);
		TExpression* GetCompare()const;
		TStatements* GetStatements()const;
	};
}