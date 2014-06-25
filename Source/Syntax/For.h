#pragma once

#include "Statement.h"

class TStatements;
class TClass;
class TExpression;

class TFor:public TStatement
{
	std::shared_ptr<TExpression> compare;
	std::shared_ptr<TStatements> increment;
	std::shared_ptr<TStatements> statements;
	void operator=(const TFor& use_source);
public:
	TFor(TClass* use_owner, TMethod* use_method, TStatements* use_parent, int use_stmt_id);
	void AnalyzeSyntax(TLexer& source);
	TFormalParam Build(TNotOptimizedProgram &program,int& local_var_offset);
	TStatement* GetCopy();
	void InitOwner(TClass* use_owner, TMethod* use_method, TStatements* use_parent);
};