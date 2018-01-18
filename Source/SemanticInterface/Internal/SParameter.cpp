#include "SParameter.h"

#include "SClass.h"
#include "SExpression.h"

#include "SMethod.h"

#include <stdexcept>

TSParameter::TSParameter(TSClass* use_owner, TSMethod* use_method, SyntaxApi::IParameter* use_syntax_node, SyntaxApi::IType* use_type_syntax_node)
	: TSyntaxNode(use_syntax_node)
	, TVariable(SemanticApi::VariableType::Parameter)
	, type(use_owner, use_type_syntax_node), owner(use_owner), method(use_method), is_ref(use_syntax_node->IsRef())
{
}

TSParameter::TSParameter(TSClass* use_owner, TSMethod* use_method, TSClass* use_class, bool use_is_ref)
	: TSyntaxNode(nullptr)
	, TVariable(SemanticApi::VariableType::Parameter)
	, type(use_owner, use_class), owner(use_owner), method(use_method), is_ref(use_is_ref)
{

}

void TSParameter::LinkBody(SemanticApi::TGlobalBuildContext build_context)
{
	type.LinkBody(build_context);
}
void TSParameter::LinkSignature(SemanticApi::TGlobalBuildContext build_context)
{
	type.LinkSignature(build_context);
}
SemanticApi::ISClass* TSParameter::GetClass()const
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
SemanticApi::TFormalParameter TSParameter::AsFormalParameter()const
{
	return SemanticApi::TFormalParameter(type.GetClass(),is_ref);
}
void TSParameter::CalculateSize()
{
	if (GetSyntax()->IsRef())
		SetSize(1);
	else
		SetSize(GetClass()->GetSize());
}

TActualParamWithConversion::TActualParamWithConversion()
{
	this->copy_constr = nullptr;
	this->conversion = nullptr;
	this->ref_to_rvalue = false;
}
void TActualParamWithConversion::BuildConvert(TExpressionResult from_result, SemanticApi::TFormalParameter to_formal_param)
{
	result = from_result;
	auto result_class = dynamic_cast<TSClass*>(result.GetClass());
	//если необходимо преобразование типа формального параметра то добавляем его
	if (result.GetClass() != to_formal_param.GetClass())
	{
		conversion = result_class->GetConversion(result.IsRef(), to_formal_param.GetClass());
		if (result.IsRef() && !to_formal_param.IsRef())
		{
			if (conversion == nullptr)
			{
				//если отсутствует преобразование, но имеется конструктор копии
				copy_constr = result_class->GetCopyConstr();
				conversion = result_class->GetConversion(false, to_formal_param.GetClass());
			}
		}
		if(conversion == nullptr)
			throw std::logic_error("Невозможно преобразовать тип!");
	}
	//если в стеке находится ссылка, а в качестве параметра требуется значение, то добавляем преобразование
	else if (result.IsRef() && !to_formal_param.IsRef())
	{
		copy_constr = result_class->GetCopyConstr();
		ref_to_rvalue = true;
	}
}

SemanticApi::ISOperations::ISOperation * TActualParamWithConversion::GetExpression() const
{
	return expression.get();
}

SemanticApi::ISMethod * TActualParamWithConversion::GetCopyConstr() const
{
	return copy_constr;
}

bool TActualParamWithConversion::IsRefToRValue() const
{
	return ref_to_rvalue;
}

SemanticApi::ISMethod * TActualParamWithConversion::GetConverstion() const
{
	return conversion;
}

const SemanticApi::IExpressionResult * TActualParamWithConversion::GetResult() const
{
	return &result;
}

void TActualParameters::Build(const std::vector<TSOperation*>& actual_params, const std::vector<SemanticApi::TFormalParameter>& formal_params)
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

std::vector<const SemanticApi::IActualParamWithConversion*> TActualParameters::GetInput() const
{
	std::vector<const SemanticApi::IActualParamWithConversion*> result;
	result.reserve(input.size());
	for (auto& v : input)
		result.push_back(&v);
	return result;
}
