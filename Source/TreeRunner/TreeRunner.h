#pragma once

#include "../SemanticInterface/SemanticTreeApi.h"

#include "FormalParam.h"
#include "RunContext.h"

class BALUSCRIPT_DLL_INTERFACE TreeRunner
{
public:
	static void RunConversion(const SemanticApi::IActualParamWithConversion* curr_op, TGlobalRunContext global_context, TStackValue &value);

	static void Construct(const SemanticApi::IActualParameters* curr_op, std::vector<TStackValue> &method_call_formal_params, TStatementRunContext run_context);
	static void Destroy(std::vector<TStackValue> &method_call_formal_params, TStatementRunContext run_context);

	static void Run(SemanticApi::ISMethod* curr_op, TMethodRunContext run_context);

	static void Run(SemanticApi::ISFor*, TStatementRunContext run_context);
	static void Run(SemanticApi::ISWhile* while_statement, TStatementRunContext run_context);
	static void Run(SemanticApi::ISReturn* return_statement, TStatementRunContext run_context);
	static void Run(SemanticApi::ISIf*, TStatementRunContext run_context);
	static void Run(SemanticApi::ISBytecode* bytecode, TStatementRunContext run_context);

	static void RunStatements(SemanticApi::ISStatements* curr_op, TStatementRunContext run_context);

	static void Construct(SemanticApi::ISConstructObject* curr_op, TStackValue& constructed_object, TStatementRunContext run_context);
	static void Destruct(SemanticApi::ISConstructObject* curr_op, TStackValue& destroyed_object, TGlobalRunContext run_context);

	static void Run(SemanticApi::ISLocalVar* local_var, TStatementRunContext run_context);
	static void Destruct(SemanticApi::ISLocalVar* local_var, TGlobalRunContext global_context, std::vector<TStackValue>& local_variables);

	static void RunAutoDefConstr(SemanticApi::ISClass* _this, TGlobalRunContext global_context, TStackValue& object);
	static void RunAutoDestr(SemanticApi::ISClass* _this, TGlobalRunContext global_context, TStackValue& object);
	static void RunFieldCopyConstr(SemanticApi::ISClassField* field, TGlobalRunContext global_context, TStackValue& source_object, TStackValue& dest_field);
	static void RunAutoCopyConstr(SemanticApi::ISClass* _this, TGlobalRunContext global_context, std::vector<TStackValue> &formal_params, TStackValue& object);
	static void RunAutoAssign(SemanticApi::ISClass* _this, TGlobalRunContext global_context, std::vector<TStackValue> &formal_params);

	static void InitializeStaticClassFields(std::vector<SemanticApi::ISClassField*> static_fields, TGlobalRunContext run_context);
	static void InitializeStaticVariables(std::vector<SemanticApi::ISLocalVar*> static_variables, TGlobalRunContext run_context);
	static void DeinitializeStatic(TGlobalRunContext run_context);
};