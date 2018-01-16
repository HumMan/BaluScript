#pragma once

#include "../SyntaxTreeApi.h"

namespace SyntaxInternal
{
	class TMethod;
	class TStatements;
	class TClass;
	class TStatementVisitor;

	class TStatement :public Lexer::TTokenPos, public virtual SyntaxApi::IStatement
	{
	private:
		TMethod* method;
		TStatements* parent;
		TClass* owner;
		int stmt_id;//порядковый номер блока в родительском блоке
		SyntaxApi::TStatementType stmt_type;
	public:
		void SetStmtId(int use_id);
		int GetStmtId()const;
		SyntaxApi::IMethod* GetMethod()const;
		TMethod* GetMethodT()const;
		SyntaxApi::IStatements* GetParent()const;
		TStatements* GetParentT()const;
		SyntaxApi::IClass* GetOwner()const;
		TClass* GetOwnerT()const;
		TStatement(SyntaxApi::TStatementType use_stmt_type, TClass* use_owner, TMethod* use_method, TStatements* use_parent, int use_stmt_id);
		SyntaxApi::TStatementType GetType()const;
		virtual ~TStatement();
	};
}