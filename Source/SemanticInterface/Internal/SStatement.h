#pragma once

#include "../../SyntaxInterface/SyntaxTreeApi.h"

#include "SSyntaxNode.h"

class TSMethod;
class TSStatements;
class TSClass;
class TStackValue;
class TExpressionResult;
class TActualParamWithConversion;

class TSStatement :public TSyntaxNode<SyntaxApi::IStatement>
{
private:
	TSMethod* method;
	TSStatements* parent;
	TSClass* owner;
public:
	TSStatements* GetParentStatements()const
	{
		return parent;
	}
	TSStatement(SyntaxApi::TStatementType use_stmt_type, TSClass* use_owner, TSMethod* use_method, TSStatements* use_parent, SyntaxApi::IStatement* use_syntax) :
		TSyntaxNode(use_syntax),
		method(use_method), parent(use_parent), owner(use_owner){}
	//virtual void Run(std::vector<TStackValue> &stack, bool& result_returned, TStackValue* return_value, int method_base) = 0;
	//virtual void Run(TStatementRunContext run_context) = 0;
	virtual ~TSStatement(){}
	TSClass* GetOwner()const
	{
		return owner;
	}
	TSMethod* GetMethod()const
	{
		return method;
	}
	void TestBoolExpr(TExpressionResult& compare_result, std::unique_ptr<TActualParamWithConversion>& conversion);
};

