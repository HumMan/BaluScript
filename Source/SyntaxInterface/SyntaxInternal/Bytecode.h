#pragma once

#include "../SyntaxTreeApi.h"

#include "Statement.h"

namespace SyntaxInternal
{
	class TClass;
	class TMethod;
	class TStatements;

	

	class TBytecode :public TStatement, public SyntaxApi::IBytecode
	{
	private:
		std::vector<TOpcode::Enum> code;
	public:
		TBytecode(TClass* use_owner, TMethod* use_method, TStatements* use_parent, int use_stmt_id);
		void Accept(SyntaxApi::IStatementVisitor* visitor);
		void AnalyzeSyntax(Lexer::ILexer* source);
		const std::vector<TOpcode::Enum>& GetBytecode()const;
	};
}