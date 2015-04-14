#pragma once

#include "SType.h"
#include "SStatement.h"
#include "Variable.h"

class TSExpression;
class TLocalVar;
class TExpressionResult;
class TSExpression_TMethodCall;
class TOperation;
class TSConstructObject;

class TSLocalVar :public TSStatement, public TVariable, public TNodeWithOffset
{
	std::unique_ptr<TSExpression> assign_expr;
	std::unique_ptr<TSConstructObject> construct_object;
	TSType type;
public:
	TLocalVar* GetSyntax()
	{
		return (TLocalVar*)TSyntaxNode::GetSyntax();
	}
	TSLocalVar(TSClass* use_owner, TSMethod* use_method, TSStatements* use_parent, TLocalVar* use_syntax);
	TNameId GetName();
	TSClass* GetClass();
	bool IsStatic();
	void Build(TGlobalBuildContext build_context);
	void Run(TStatementRunContext run_context);
	void Destruct(std::vector<TStaticValue> &static_fields, std::vector<TStackValue>& local_variables);
};