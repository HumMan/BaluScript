#pragma once

#include "Statement.h"
#include "Variable.h"
#include "Type.h"

class TExpression;

class TLocalVar:public TStatement,public TVariable
{
	TType type;
	TNameId name;
	TExpression* assign_expr;
	TVectorList<TExpression> params;
	int offset;
	bool is_static;
	void operator=(const TLocalVar& use_source);
public:
	TLocalVar(TClass* use_owner, TMethod* use_method, TStatements* use_parent, int use_stmt_id);
	void AnalyzeSyntax(TLexer& source);
	TNameId GetName();
	TFormalParam Build(TNotOptimizedProgram &program,int& local_var_offset);
	~TLocalVar();
	TClass* GetClass();
	int GetOffset();
	bool IsStatic();
	TFormalParam PushRefToStack(TNotOptimizedProgram &program);
	TStatement* GetCopy();
	void InitOwner(TClass* use_owner, TMethod* use_method, TStatements* use_parent);

};