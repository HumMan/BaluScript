#pragma once

#include "../SyntaxTree/SyntaxTreeApi.h"

#include "Statement.h"
#include "Type.h"

namespace SyntaxInternal
{
	class TExpression;

	class TLocalVar :public TStatement, public virtual SyntaxApi::ILocalVar
	{
		std::shared_ptr<TType> type;
		Lexer::TNameId name;
		std::unique_ptr<TExpression> assign_expr;
		std::vector<std::unique_ptr<TExpression>> params;
		bool is_static;
	public:
		SyntaxApi::IExpression* GetAssignExpr()const;
		std::vector<SyntaxApi::IExpression*> GetParams()const;
		TLocalVar(TClass* use_owner, TMethod* use_method, TStatements* use_parent, int use_stmt_id);
		void AnalyzeSyntax(Lexer::ILexer* source);
		Lexer::TNameId GetName()const;
		bool IsStatic();
		TType* GetVarType()
		{
			return type.get();
		}
		void Accept(SyntaxApi::IStatementVisitor* visitor);
	};
}