#pragma once

#include "Statement.h"

class TFor:public TStatement
{
	TExpression compare;
	TStatements increment;
	TStatements statements;
	void operator=(const TFor& use_source);
public:
	TFor(TClass* use_owner,TMethod* use_method,TStatements* use_parent,int use_stmt_id)
		:TStatement(TStatementType::For,use_owner,use_method,use_parent,use_stmt_id)
		,compare(use_owner,use_method,use_parent,use_stmt_id)
		,increment(use_owner,use_method,use_parent,use_stmt_id)
		,statements(use_owner,use_method,use_parent,use_stmt_id)
	{}
	void AnalyzeSyntax(TLexer& source);
	TFormalParam Build(TNotOptimizedProgram &program,int& local_var_offset);
	TStatement* GetCopy()
	{
		return new TFor(*this);
	}
	void InitOwner(TClass* use_owner,TMethod* use_method,TStatements* use_parent)
	{
		TStatement::_InitOwner(use_owner,use_method,use_parent);
		compare.InitOwner(use_owner,use_method,use_parent);
		increment.InitOwner(use_owner,use_method,use_parent);
		statements.InitOwner(use_owner,use_method,use_parent);
	}
};