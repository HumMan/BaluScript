#include "RunContext.h"

#include "FormalParam.h"

void TMethodRunContext::AddRefsFromParams()
{
	if (formal_params == nullptr)
		return;	
	for (auto& p : *formal_params)
	{
		if (p.IsRef())
			refs_list->AddRef(p.get());
	}
}
void TMethodRunContext::RemoveRefsFromParams()
{
	if (formal_params == nullptr)
		return;
	for (auto& p : *formal_params)
	{
		if (p.IsRef())
			refs_list->RemoveRef(p.get());
	}
}

void TExpressionRunContext::SetExpressionResult(const TStackValue& value)
{
	assert(expression_result->get() == nullptr);
	*expression_result = value;
}

void TExpressionRunContext::SetExpressionResult(TStackValue* &value)
{
	assert(expression_result->get() == nullptr);
	if (value->IsRef())
	{
		//если это просто ссылка то записываем её на новое место, данные не трогаем
		*expression_result = *value;
		delete value;
	}
	else
	{
		//если это значение, то записываем его в список временных объектов, а дальше передаём ссылку на него
		*expression_result = TStackValue(true, value->GetClass());
		expression_result->SetAsReference(value->get());
		temp_objects->push_back(value);
	}
}

TStackValue* TExpressionRunContext::GetExpressionResult()
{
	return expression_result;
}