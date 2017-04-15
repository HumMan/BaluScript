#pragma once

#include "../SyntaxTree/SyntaxTreeApi.h"

#include "Statement.h"
#include "Expression.h"

namespace SyntaxInternal
{
	class TReturn :public TStatement, public SyntaxApi::IReturn
	{
		std::unique_ptr<TExpression> result;
		void operator=(const TReturn& use_source);
	public:
		TExpression* GetResult()const;
		TReturn(TClass* use_owner, TMethod* use_method, TStatements* use_parent, int use_stmt_id)
			:TStatement(SyntaxApi::TStatementType::Return, use_owner, use_method, use_parent, use_stmt_id)//TODO вроде здесь родитель и id не нужен вовсе
			, result(new TExpression(use_owner, use_method, use_parent, use_stmt_id))
		{}
		void AnalyzeSyntax(Lexer::ILexer* source);
		void Accept(SyntaxApi::IStatementVisitor* visitor);
	};
}