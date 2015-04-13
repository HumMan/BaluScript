
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

void TSConstructObject::Build(TTokenPos* source, std::vector<TExpressionResult>& params_result, std::vector<TSOperation*>& params, std::vector<TFormalParameter>& params_formals, std::vector<TSClassField*>* static_fields, std::vector<TSLocalVar*>* static_variables)
{
	TSMethod* constructor = NULL;
	if (params_result.size() > 0)
	{
		std::vector<TSMethod*> constructors;
		object_type->GetCopyConstructors(constructors);
		constructor = FindMethod(source, constructors, params_result);
		
		if (constructor == NULL)
			source->Error(" оструктора с такими парметрами не существует!");
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

void TSConstructObject::Build(TTokenPos* source, std::vector<std::unique_ptr<TExpression>>& syntax_params, std::vector<TSClassField*>* static_fields, std::vector<TSLocalVar*>* static_variables)
{
	std::vector<TExpressionResult> params_result;
	std::vector<TFormalParameter> params_formals;
	std::vector<TSOperation*> params;

	for (const std::unique_ptr<TExpression>& param_syntax : syntax_params)
	{
		auto t = new TSExpression(owner, method, parent, param_syntax.get());
		t->Build(static_fields, static_variables);
		params.push_back(t);
		params_result.push_back(params.back()->GetFormalParameter());
		params_formals.push_back(TFormalParameter(params_result.back().GetClass(), params_result.back().IsRef()));
	}
	Build(source, params_result, params, params_formals, static_fields, static_variables);
}


void TSConstructObject::Construct(TStackValue& constructed_object, std::vector<TStaticValue> &static_fields, std::vector<TStackValue> &formal_params, TStackValue& object, std::vector<TStackValue>& local_variables)
{
	if (constructor_call)
	{
		TStackValue without_result;
		constructor_call->Run(static_fields, formal_params, without_result, constructed_object, local_variables);
	}
}

void TSConstructObject::Destruct(TStackValue& constructed_object, std::vector<TStaticValue> &static_fields, std::vector<TStackValue>& local_variables)
{
	TSMethod* destr = object_type->GetDestructor();
	if (destr != NULL)
	{
		std::vector<TStackValue> without_params;
		TStackValue without_result;

		destr->Run(static_fields, without_params, without_result, constructed_object);
	}
}