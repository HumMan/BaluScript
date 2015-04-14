#pragma once

#include <memory>

#include "../lexer.h"

#include "../Syntax/Statement.h"

#include "SSyntaxNode.h"

#include "RunContext.h"

class TSMethod;
class TSStatements;
class TSClass;
class TStackValue;
class TExpressionResult;
class TActualParamWithConversion;
class TStaticValue;

class TSStatement :public TSyntaxNode<TStatement>
{
protected:
	TSMethod* method;
	TSStatements* parent;
	TSClass* owner;
public:
	TSStatements* GetParentStatements()
	{
		return parent;
	}
	TSStatement(TStatementType::Enum use_stmt_type, TSClass* use_owner, TSMethod* use_method, TSStatements* use_parent, TStatement* use_syntax) :
		method(use_method), parent(use_parent), owner(use_owner),
		TSyntaxNode(use_syntax){}
	//virtual void Run(std::vector<TStackValue> &stack, bool& result_returned, TStackValue* return_value, int method_base) = 0;
	virtual void Run(TStatementRunContext run_context) = 0;
	virtual ~TSStatement(){}
	void TestBoolExpr(TExpressionResult& compare_result, std::unique_ptr<TActualParamWithConversion>& conversion);
};

