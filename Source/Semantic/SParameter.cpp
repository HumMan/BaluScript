#include "SParameter.h"

#include "../Syntax/Statements.h"
#include "../Syntax/Method.h"

#include "SClass.h"
#include "SExpression.h"

#include <stdexcept>

TSParameter::TSParameter(TSClass* use_owner, TSMethod* use_method, SyntaxApi::IParameter* use_syntax_node, SyntaxApi::IType* use_type_syntax_node)
	: TSyntaxNode(use_syntax_node)
	, TVariable(TVariableType::Parameter)
	, type(use_owner, use_type_syntax_node), owner(use_owner), method(use_method), is_ref(use_syntax_node->IsRef())
{
}

TSParameter::TSParameter(TSClass* use_owner, TSMethod* use_method, TSClass* use_class, bool use_is_ref)
	: TSyntaxNode(NULL)
	, TVariable(TVariableType::Parameter)
	, type(use_owner, use_class), owner(use_owner), method(use_method), is_ref(use_is_ref)
{

}

void TSParameter::LinkBody(TGlobalBuildContext build_context)
{
	type.LinkBody(build_context);
}
void TSParameter::LinkSignature(TGlobalBuildContext build_context)
{
	type.LinkSignature(build_context);
}
TSClass* TSParameter::GetClass()
{
	return type.GetClass();
}

bool TSParameter::IsEqualTo(const TSParameter& right)const
{
	return type.IsEqualTo(right.type) && GetSyntax()->IsRef() == right.GetSyntax()->IsRef();
}

bool TSParameter::IsRef()const
{
	return is_ref;
}
TFormalParameter TSParameter::AsFormalParameter()const
{
	return TFormalParameter(type.GetClass(),is_ref);
}
void TSParameter::CalculateSize()
{
	if (GetSyntax()->IsRef())
		SetSize(1);
	else
		SetSize(GetClass()->GetSize());
}

void TActualParamWithConversion::RunConversion(std::vector<TStaticValue> &static_fields, TStackValue &value)
{
	if (ref_to_rvalue)
	{
		assert(copy_constr != NULL);
		//if (copy_constr != NULL)
		{
			std::vector<TStackValue> constr_params;
			constr_params.push_back(value);
			TStackValue constr_result;
			TStackValue constructed_object(false, value.GetClass());
			copy_constr->Run(TMethodRunContext(&static_fields, &constr_params, &constr_result, &constructed_object));
			value = constructed_object;
		}
		//else
		//{
		//	TStackValue constructed_object(false, value.GetClass());
		//	memcpy(constructed_object.get(), value.get(), value.GetClass()->GetSize()*sizeof(int));
		//	value = constructed_object;
		//}
	}
	if (conversion != NULL)
	{
		std::vector<TStackValue> conv_params;
		conv_params.push_back(value);
		TStackValue result;
		conversion->Run(TMethodRunContext(&static_fields, &conv_params, &result, &value));
		value = result;
	}
}

TActualParamWithConversion::TActualParamWithConversion()
{
	this->copy_constr = NULL;
	this->conversion = NULL;
	this->ref_to_rvalue = false;
}
void TActualParamWithConversion::BuildConvert(TExpressionResult from_result, TFormalParameter to_formal_param)
{
	result = from_result;

	//если необходимо преобразование типа формального параметра то добавляем его
	if (result.GetClass() != to_formal_param.GetClass())
	{
		conversion = result.GetClass()->GetConversion(result.IsRef(), to_formal_param.GetClass());
		if (result.IsRef() && !to_formal_param.IsRef())
		{
			if (conversion == NULL)
			{
				//если отсутствует преобразование, но имеется конструктор копии
				copy_constr = result.GetClass()->GetCopyConstr();
				conversion = result.GetClass()->GetConversion(false, to_formal_param.GetClass());
			}
		}
		if(conversion == NULL)
			throw std::logic_error("Невозможно преобразовать тип!");
	}
	//если в стеке находится ссылка, а в качестве параметра требуется значение, то добавляем преобразование
	else if (result.IsRef() && !to_formal_param.IsRef())
	{
		copy_constr = result.GetClass()->GetCopyConstr();
		ref_to_rvalue = true;
	}
}

void TActualParameters::Build(const std::vector<TSOperation*>& actual_params, const std::vector<TFormalParameter>& formal_params)
{
	assert(actual_params.size() == formal_params.size());
	int i = 0;
	for (TSOperation* act : actual_params)
	{
		input.emplace_back();
		input.back().expression.reset(act);
		input.back().BuildConvert(act->GetFormalParameter(), formal_params[i]);
		i++;
	}
}

void TActualParameters::Construct(std::vector<TStackValue> &method_call_formal_params, TStatementRunContext run_context)
{
	for (TActualParamWithConversion& par : input)
	{
		TStackValue exp_result;
		par.expression->Run(TExpressionRunContext(run_context, &exp_result));

		par.RunConversion(*run_context.static_fields, exp_result);
		method_call_formal_params.push_back(exp_result);
	}
}

void TActualParameters::Destroy(std::vector<TStackValue> &method_call_formal_params, TStatementRunContext run_context)
{
	auto it = input.begin();
	for (size_t i = 0; i < method_call_formal_params.size(); i++)
	{
		if (!it->result.IsRef() && it->result.GetClass()->GetDestructor() != NULL)
		{
			TStackValue destructor_result;
			std::vector<TStackValue> without_params;
			it->result.GetClass()->GetDestructor()->Run(TMethodRunContext(run_context.static_fields, &without_params, &destructor_result, &method_call_formal_params[i]));
		}
		it++;
	}
}
