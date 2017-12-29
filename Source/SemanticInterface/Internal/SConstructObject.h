#pragma once

#include "SType.h"
#include "SStatement.h"
#include "Variable.h"
#include "SParameter.h"

class TExpressionResult;
class TSExpression_TMethodCall;
class TSOperation;

class TSConstructObject
{
	std::unique_ptr<TSExpression_TMethodCall> constructor_call;
	SemanticApi::ISClass* object_type;
	SemanticApi::ISMethod* method;
	TSStatements* parent;
	SemanticApi::ISClass* owner;
public:
	TSConstructObject(SemanticApi::ISClass* use_owner, SemanticApi::ISMethod* use_method, TSStatements* use_parent, SemanticApi::ISClass* object_type);
	void Build(Lexer::ITokenPos* source, std::vector<TExpressionResult>& exp_results, std::vector<TSOperation*>& params, std::vector<SemanticApi::TFormalParameter>& params_formals, SemanticApi::TGlobalBuildContext build_context);
	void Build(Lexer::ITokenPos* source, const std::vector<SyntaxApi::IExpression*>& params, SemanticApi::TGlobalBuildContext build_context);
	/*void Construct(TStackValue& constructed_object, TStatementRunContext run_context);
	void Destruct(TStackValue& destroyed_object, TGlobalRunContext run_context);*/
};