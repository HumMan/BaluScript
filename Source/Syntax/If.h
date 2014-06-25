#pragma once

#include "Statement.h"

class TIf:public TStatement
{
	TExpression bool_expr;
	TStatements statements,else_statements;
	void operator=(const TIf& use_source);
public:
	TIf(TClass* use_owner,TMethod* use_method,TStatements* use_parent,int use_stmt_id)
		:TStatement(TStatementType::If,use_owner,use_method,use_parent,use_stmt_id)
		,bool_expr(use_owner,use_method,use_parent,use_stmt_id)
		,statements(use_owner,use_method,use_parent,use_stmt_id)
		,else_statements(use_owner,use_method,use_parent,use_stmt_id)
	{}
	void AnalyzeSyntax(TLexer& source);
	TFormalParam Build(TNotOptimizedProgram &program,int& local_var_offset);
	TStatement* GetCopy()
	{
		return new TIf(*this);
	}
	void InitOwner(TClass* use_owner,TMethod* use_method,TStatements* use_parent)
	{
		TStatement::_InitOwner(use_owner,use_method,use_parent);
		bool_expr.InitOwner(use_owner,use_method,use_parent);
		statements.InitOwner(use_owner,use_method,use_parent);
		else_statements.InitOwner(use_owner,use_method,use_parent);
	}
};