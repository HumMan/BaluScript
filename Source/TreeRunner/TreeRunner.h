#pragma once

#include "../SemanticInterface/SemanticTreeApi.h"

#include "FormalParam.h"
#include "RunContext.h"

class TreeRunner
{
public:
	//void RunConversion(SemanticApi::IActualParamWithConversion* curr_op, std::vector<TStaticValue> &static_fields, TStackValue &value);

	static void Construct(const SemanticApi::IActualParameters* curr_op, std::vector<TStackValue> &method_call_formal_params, TStatementRunContext run_context);
	static void Destroy(const SemanticApi::IActualParameters* curr_op, std::vector<TStackValue> &method_call_formal_params, TStatementRunContext run_context);

	static void Run(SemanticApi::ISMethod* curr_op, TMethodRunContext run_context);

	static void Run(SemanticApi::ISFor, TStatementRunContext run_context);

	//void Run(SemanticApi::ISStatements* curr_op, , TStatementRunContext run_context);

	static void Construct(SemanticApi::ISConstructObject* curr_op, TStackValue& constructed_object, TStatementRunContext run_context);
	static void Destruct(SemanticApi::ISConstructObject* curr_op, TStackValue& destroyed_object, TGlobalRunContext run_context);

	//void Run(SemanticApi::ISLocalVar, TStatementRunContext run_context);
	//void Destruct(SemanticApi::ISLocalVar, std::vector<TStaticValue> &static_fields, std::vector<TStackValue>& local_variables);
};


void InitializeStaticClassFields(std::vector<SemanticApi::ISClassField*> static_fields, std::vector<TStaticValue> &static_objects);
void InitializeStaticVariables(std::vector<SemanticApi::ISLocalVar*> static_variables, std::vector<TStaticValue> &static_objects);