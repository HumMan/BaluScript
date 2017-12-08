
#include "SConstructObject.h"

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

void TSConstructObject::Build(Lexer::ITokenPos* source, std::vector<TExpressionResult>& params_result, std::vector<TSOperation*>& params, std::vector<TFormalParameter>& params_formals, TGlobalBuildContext build_context)
{
	TSMethod* constructor = nullptr;
	if (params_result.size() > 0)
	{
		std::vector<TSMethod*> constructors;
		object_type->GetCopyConstructors(constructors);
		constructor = FindMethod(source, constructors, params_result);
		
		if (constructor == nullptr)
			source->Error(" оструктора с такими парметрами не существует!");
	}
	else
	{
		constructor = object_type->GetDefConstr();
	}

	TSMethod* destructor = object_type->GetDestructor();
	if (destructor != nullptr)
	{
		ValidateAccess(source, owner, destructor);
	}

	if (constructor != nullptr)
	{
		constructor_call.reset(new TSExpression_TMethodCall(TSExpression_TMethodCall::ObjectConstructor));
		constructor_call->Build(params, constructor);
		ValidateAccess(source, owner, constructor);
	}
}

void TSConstructObject::Build(Lexer::ITokenPos* source, const std::vector<SyntaxApi::IExpression*>& syntax_params, TGlobalBuildContext build_context)
{
	std::vector<TExpressionResult> params_result;
	std::vector<TFormalParameter> params_formals;
	std::vector<TSOperation*> params;

	for (auto param_syntax : syntax_params)
	{
		auto t = new TSExpression(owner, method, parent, param_syntax);
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
		TStatementRunContext constr_run_context(run_context);
		constr_run_context.object = &constructed_object;
		constructor_call->Run(TExpressionRunContext(constr_run_context, nullptr));
	}
}

void TSConstructObject::Destruct(TStackValue& constructed_object, TGlobalRunContext run_context)
{
	TSMethod* destr = object_type->GetDestructor();
	if (destr != nullptr)
	{
		destr->Run(TMethodRunContext(run_context.static_fields, nullptr, nullptr, &constructed_object));
	}
}