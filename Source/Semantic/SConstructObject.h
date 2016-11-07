#pragma once

#include "SType.h"
#include "SStatement.h"
#include "Variable.h"
#include "SParameter.h"

class TSExpression;
class TExpressionResult;
class TSExpression_TMethodCall;
class TSOperation;

class TSConstructObject
{
	std::unique_ptr<TSExpression_TMethodCall> constructor_call;
	TSClass* object_type;
	TSMethod* method;
	TSStatements* parent;
	TSClass* owner;
public:
	TSConstructObject(TSClass* use_owner, TSMethod* use_method, TSStatements* use_parent, TSClass* object_type);
	void Build(Lexer::TTokenPos* source, std::vector<TExpressionResult>& exp_results, std::vector<TSOperation*>& params, std::vector<TFormalParameter>& params_formals, TGlobalBuildContext build_context);
	void Build(Lexer::TTokenPos* source, std::vector<std::unique_ptr<TExpression>>& params, TGlobalBuildContext build_context);
	void Construct(TStackValue& constructed_object, TStatementRunContext run_context);
	void Destruct(TStackValue& destroyed_object, TGlobalRunContext run_context);
};