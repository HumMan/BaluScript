
#include "SConstructObject.h"

#include "../Syntax/Statements.h"

#include "../semanticAnalyzer.h"

#include "FormalParam.h"
#include "SExpression.h"
#include "SClass.h"
#include "SLocalVar.h"
#include "SStatement.h"
#include "SParameter.h"

TSConstructObject::TSConstructObject(TSClass* use_owner, TSMethod* use_method, TSStatements* use_parent, TSClass* use_object_type)
	:owner(use_owner), method(use_method), parent(use_parent), object_type(use_object_type)
{
}

void TSConstructObject::Build(TTokenPos* source, std::vector<TExpressionResult>& params_result, std::vector<TSOperation*>& params, std::vector<TFormalParameter>& params_formals, TGlobalBuildContext build_context)
{
	TSMethod* constructor = NULL;
	if (params_result.size() > 0)
	{
		std::vector<TSMethod*> constructors;
		object_type->GetCopyConstructors(constructors);
		constructor = FindMethod(source, constructors, params_result);
		
		if (constructor == NULL)
			source->Error("����������� � ������ ���������� �� ����������!");
	}
	else
	{
		constructor = object_type->GetDefConstr();
	}

	TSMethod* destructor = object_type->GetDestructor();
	if (destructor != NULL)
	{
		ValidateAccess(source, owner, destructor);
	}

	if (constructor != NULL)
	{
		constructor_call.reset(new TSExpression_TMethodCall(TSExpression_TMethodCall::ObjectConstructor));
		constructor_call->Build(params, constructor);
		ValidateAccess(source, owner, constructor);
	}
}

void TSConstructObject::Build(TTokenPos* source, std::vector<std::unique_ptr<TExpression>>& syntax_params, TGlobalBuildContext build_context)
{
	std::vector<TExpressionResult> params_result;
	std::vector<TFormalParameter> params_formals;
	std::vector<TSOperation*> params;

	for (const std::unique_ptr<TExpression>& param_syntax : syntax_params)
	{
		auto t = new TSExpression(owner, method, parent, param_syntax.get());
		t->Build(build_context);
		params.push_back(t);
		params_result.push_back(params.back()->GetFormalParameter());
		params_formals.push_back(TFormalParameter(params_result.back().GetClass(), params_result.back().IsRef()));
	}
	Build(source, params_result, params, params_formals, build_context);
}


void TSConstructObject::Construct(TStackValue& constructed_object, TStatementRunContext run_context)
{
	if (constructor_call)
	{
		constructor_call->Run(TExpressionRunContext(run_context, nullptr));
	}
}

void TSConstructObject::Destruct(TStackValue& constructed_object, TGlobalRunContext run_context)
{
	TSMethod* destr = object_type->GetDestructor();
	if (destr != NULL)
	{
		TMethodRunContext destructor_run_context;
		*(TGlobalRunContext*)&destructor_run_context = run_context;
		destr->Run(destructor_run_context);
	}
}