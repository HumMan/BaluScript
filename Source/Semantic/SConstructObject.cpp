
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
	constr_copy_memcpy = false;
}

void TSConstructObject::Build(TTokenPos* source, std::vector<TExpressionResult>& params_result, std::vector<TSOperation*>& params, std::vector<TFormalParameter>& params_formals, std::vector<TSClassField*>* static_fields, std::vector<TSLocalVar*>* static_variables)
{
	int conv_need = 0;

	TSMethod* constructor = NULL;
	if (params_result.size() > 0)
	{
		std::vector<TSMethod*> constructors;
		object_type->GetCopyConstructors(constructors);
		constructor = FindMethod(source, constructors, params_result, conv_need);
		
		if (constructor == NULL)
		{
			if (params_result.size() == 1 && params_result[0].GetClass() == object_type)
			{
				//TODO если имеется конструктор с использованием conversion простое копирование памяти не будет выполнено
				constr_copy_memcpy = true;
			}
			else
				source->Error("Коструктора с такими парметрами не существует!");
		}
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
	else
	{
		if (constr_copy_memcpy)
		{
			constructor_call.reset(new TSExpression_TMethodCall(TSExpression_TMethodCall::ObjectConstructorInitWithAssign));
			constructor_call->Build(params, constructor);
		}
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
	//if (constr_copy_memcpy)
	//{
	//	std::vector < TStackValue > method_call_params;
	//	//actual_params.Construct(method_call_params, static_fields, formal_params, object, local_variables);
	//	assert(method_call_params.size() == 1);
	//	memcpy(constructed_object.get(), method_call_params[0].get(), object_type->GetSize()*sizeof(int));		
	//}
	//else
		if (constructor_call)
	{
		constructor_call->Run(constructed_object,static_fields, formal_params, constructed_object, constructed_object, local_variables);
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