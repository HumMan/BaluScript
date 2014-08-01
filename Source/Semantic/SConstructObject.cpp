
#include "SConstructObject.h"

#include "../Syntax/Statements.h"

#include "../semanticAnalyzer.h"

#include "FormalParam.h"
#include "SExpression.h"
#include "SClass.h"
#include "SLocalVar.h"
#include "SStatement.h"

TSConstructObject::TSConstructObject(TSClass* use_owner, TSMethod* use_method, TSStatements* use_parent, TSClass* use_object_type)
	:owner(use_owner), method(use_method), parent(use_parent), object_type(use_object_type)
{
	constr_copy_memcpy = false;
}

TFormalParam TSConstructObject::Build(TSLocalVar* local_var, TTokenPos* source, std::vector<std::unique_ptr<TExpression>>& syntax_params, std::vector<TSClassField*>* static_fields, std::vector<TSLocalVar*>* static_variables)
{
	std::vector<TFormalParam> params_result;

	for (const std::unique_ptr<TExpression>& param_syntax : syntax_params)
	{
		auto t = new TSExpression(owner, method, parent, param_syntax.get());
		t->Build(static_fields, static_variables);
		params.push_back(std::unique_ptr<TSExpression>(t));
		params_result.push_back(params.back()->GetFormalParameter());
	}

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
				//constructor = memcpy
			}else
				source->Error(" оструктора с такими парметрами не существует!");
		}
	}
	else
	{
		constructor = object_type->GetDefConstr();
	}

	//bool need_testandget=GetSyntax()->is_static&&(params_result.size()>0||assign_expr!=NULL);

	TSMethod* destructor = object_type->GetDestructor();
	if (destructor != NULL)
	{
		ValidateAccess(source, owner, destructor);
		//program.Push(TOp(TOpcode::PUSH_GLOBAL_REF,offset),program.static_vars_destroy);
		//program.static_vars_destroy+=destructor->BuildCall(program,params_result).GetOps();
	}

	if (constructor != NULL)
	{
		std::vector<TOperation*> param_expressions;
		for (const std::unique_ptr<TOperation>& v : params)
		{
			param_expressions.push_back(v.get());
		}
		constructor_call = std::unique_ptr<TSExpression_TMethodCall>(new TSExpression_TMethodCall());

		TSExpression::TGetLocal* get_local_id = new TSExpression::TGetLocal();
		get_local_id->variable = local_var;
		constructor_call->left = get_local_id;
		constructor_call->Build(param_expressions, params_result, constructor);
		ValidateAccess(source, owner, constructor);
	}
	return TVoid();
}


void TSConstructObject::Construct(TStackValue& constructed_object, std::vector<TStaticValue> &static_fields, std::vector<TStackValue> &formal_params, bool& result_returned, TStackValue& result, TStackValue& object, std::vector<TStackValue>& local_variables)
{
	if (constructor_call)
	{
		TStackValue constr_result;
		//constructor_call->Run(formal_params, constr_result, local_variables[GetOffset()], local_variables);
		constructor_call->Run(static_fields, formal_params, constr_result, object, local_variables);
	}
	//вызываем конструктор по умолчанию
	else
	{
		TSMethod* def_constr = object_type->GetDefConstr();
		if (def_constr != NULL)
		{
			std::vector<TStackValue> constr_formal_params;
			TStackValue without_result;
			def_constr->Run(static_fields, constr_formal_params, without_result, constructed_object);
		}
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