
#include "SConstructObject.h"

#include "SCommon.h"

#include "SExpression.h"
#include "SClass.h"
#include "SLocalVar.h"
#include "SStatement.h"
#include "SParameter.h"

TSConstructObject::TSConstructObject(SemanticApi::ISClass* use_owner, SemanticApi::ISMethod* use_method, TSStatements* use_parent, SemanticApi::ISClass* use_object_type)
	:owner(use_owner), method(use_method), parent(use_parent), object_type(use_object_type)
{
}

void TSConstructObject::Build(Lexer::ITokenPos* source, std::vector<TExpressionResult>& params_result, std::vector<TSOperation*>& params, std::vector<SemanticApi::TFormalParameter>& params_formals, TGlobalBuildContext build_context)
{
	SemanticApi::ISMethod* constructor = nullptr;
	if (params_result.size() > 0)
	{
		std::vector<SemanticApi::ISMethod*> constructors;
		object_type->GetCopyConstructors(constructors);
		constructor = FindMethod(source, constructors, params_result);
		
		if (constructor == nullptr)
			source->Error(" оструктора с такими парметрами не существует!");
	}
	else
	{
		constructor = object_type->GetDefConstr();
	}

	SemanticApi::ISMethod* destructor = object_type->GetDestructor();
	if (destructor != nullptr)
	{
		ValidateAccess(source, owner, destructor);
	}

	if (constructor != nullptr)
	{
		constructor_call.reset(new TSExpression_TMethodCall(TSExpression_TMethodCall::ObjectConstructor));
		constructor_call->Build(params, dynamic_cast<TSMethod*>(constructor));
		ValidateAccess(source, owner, constructor);
	}
}

void TSConstructObject::Build(Lexer::ITokenPos* source, const std::vector<SyntaxApi::IExpression*>& syntax_params, TGlobalBuildContext build_context)
{
	std::vector<TExpressionResult> params_result;
	std::vector<SemanticApi::TFormalParameter> params_formals;
	std::vector<TSOperation*> params;

	for (auto param_syntax : syntax_params)
	{
		auto t = new TSExpression(dynamic_cast<TSClass*>(owner), dynamic_cast<TSMethod*>(method), parent, param_syntax);
		t->Build(build_context);
		params.push_back(t);
		params_result.push_back(params.back()->GetFormalParameter());
		params_formals.push_back(SemanticApi::TFormalParameter(params_result.back().GetClass(), params_result.back().IsRef()));
	}
	Build(source, params_result, params, params_formals, build_context);
}


