#pragma once

#include "Statement.h"
#include "For.h"
#include "Expression.h"

class TWhile:public TStatement
{
	TExpression compare;
	TStatements statements;
	void operator=(const TFor& use_source);
public:
	TWhile(TClass* use_owner,TMethod* use_method,TStatements* use_parent,int use_stmt_id)
		:TStatement(TStatementType::While,use_owner,use_method,use_parent,use_stmt_id)
		,compare(use_owner,use_method,use_parent,use_stmt_id)
		,statements(use_owner,use_method,use_parent,use_stmt_id)
	{}
	void AnalyzeSyntax(TLexer& source);
	TFormalParam Build(TNotOptimizedProgram &program,int& local_var_offset);
	TStatement* GetCopy()
	{
		return new TWhile(*this);
	}
	void InitOwner(TClass* use_owner,TMethod* use_method,TStatements* use_parent)
	{
		TStatement::_InitOwner(use_owner,use_method,use_parent);
		compare.InitOwner(use_owner,use_method,use_parent);
		statements.InitOwner(use_owner,use_method,use_parent);
	}
};