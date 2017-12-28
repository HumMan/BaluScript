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
		std::vector<SyntaxApi::TBytecodeOp> code;
		void operator=(const TBytecode& use_source);
	public:
		TBytecode(TClass* use_owner, TMethod* use_method, TStatements* use_parent, int use_stmt_id);
		void Accept(SyntaxApi::IStatementVisitor* visitor);
		void AnalyzeSyntax(Lexer::ILexer* source);
		const std::vector<SyntaxApi::TBytecodeOp>& GetBytecode()const;
	};
}