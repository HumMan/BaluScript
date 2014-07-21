#pragma once

#include "SType.h"
#include "SStatement.h"
#include "Variable.h"

class TSExpression;
class TLocalVar;
class TFormalParam;
class TSExpression_TMethodCall;
class TOperation;

class TSLocalVar :public TSStatement, public TVariable, public TNodeWithOffset
{
	TSType type;
	std::unique_ptr<TSExpression> assign_expr;
	std::vector<std::unique_ptr<TOperation>> params;
	std::unique_ptr<TSExpression_TMethodCall> constructor_call;
public:
	TLocalVar* GetSyntax()
	{
		return (TLocalVar*)TSyntaxNode::GetSyntax();
	}
	TSLocalVar(TSClass* use_owner, TSMethod* use_method, TSStatements* use_parent, TLocalVar* use_syntax);
	TNameId GetName();
	TSClass* GetClass();
	bool IsStatic();
	TFormalParam Build(std::vector<TSClassField*>* static_fields, std::vector<TSLocalVar*>* static_variables);
	void Run(std::vector<TStaticValue> &static_fields, std::vector<TStackValue> &formal_params, bool& result_returned, TStackValue& result, TStackValue& object, std::vector<TStackValue>& local_variables);
	void Destruct(std::vector<TStaticValue> &static_fields, std::vector<TStackValue>& local_variables);
};