#pragma once

#include "Statement.h"
#include "Expression.h"

class TReturn:public TStatement
{
	TExpression result;
	void operator=(const TReturn& use_source);
public:
	TReturn(TClass* use_owner,TMethod* use_method,TStatements* use_parent,int use_stmt_id)
		:TStatement(TStatementType::Return,use_owner,use_method,use_parent,use_stmt_id)//TODO вроде здесь родитель и id не нужен вовсе
		,result(use_owner,use_method,use_parent,use_stmt_id)
	{}
	void AnalyzeSyntax(TLexer& source);
	void Accept(TStatementVisitor* visitor);
};