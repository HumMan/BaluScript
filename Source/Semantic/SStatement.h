#pragma once

#include <memory>

#include "../lexer.h"

#include "../Syntax/Statement.h"

#include "SSyntaxNode.h"

class TSMethod;
class TSStatements;
class TSClass;
class TStackValue;
class TFormalParam;
class TFormalParamWithConversions;
class TStaticValue;

class TSStatement :public TSyntaxNode<TStatement>
{
protected:
	TSMethod* method;
	TSStatements* parent;
	TSClass* owner;
public:
	TSStatement(TStatementType::Enum use_stmt_type, TSClass* use_owner, TSMethod* use_method, TSStatements* use_parent, TStatement* use_syntax) :
		method(use_method), parent(use_parent), owner(use_owner),
		TSyntaxNode(use_syntax){}
	//virtual void Run(std::vector<TStackValue> &stack, bool& result_returned, TStackValue* return_value, int method_base) = 0;
	virtual void Run(std::vector<TStaticValue> &static_fields, std::vector<TStackValue> &formal_params, bool& result_returned, TStackValue& result, TStackValue& object, std::vector<TStackValue>& local_variables) = 0;
	virtual ~TSStatement(){}
	void TestBoolExpr(TFormalParam& compare_result, std::unique_ptr<TFormalParamWithConversions>& conversion);
};

