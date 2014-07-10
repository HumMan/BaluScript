#pragma once

#include "SType.h"
#include "SStatement.h"
#include "Variable.h"

class TSExpression;
class TLocalVar;
class TFormalParam;

class TSLocalVar :public TSStatement, public TVariable, public TNodeWithOffset
{
	TSType type;
	std::shared_ptr<TSExpression> assign_expr;
	std::vector<std::shared_ptr<TSExpression>> params;
public:
	TLocalVar* GetSyntax()
	{
		return (TLocalVar*)TSyntaxNode::GetSyntax();
	}
	TSLocalVar(TSClass* use_owner, TSMethod* use_method, TSStatements* use_parent, TLocalVar* use_syntax);
	TNameId GetName();
	TSClass* GetClass();
	bool IsStatic();
	TFormalParam Build();
	void Run(std::vector<TStackValue> &formal_params, bool& result_returned, TStackValue& result, TStackValue& object, std::vector<TStackValue>& local_variables);
	void Destruct();
};