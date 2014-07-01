#include "SOverloadedMethod.h"

#include "SMethod.h"
#include "../Syntax/Method.h"


TSMethod* TSOverloadedMethod::FindParams(std::vector<std::shared_ptr<TSParameter>>& params)
{
	for (const std::shared_ptr<TSMethod>& method : methods)
		if (method->HasParams(params))
			return method.get();
	return NULL;
}

TSMethod* TSOverloadedMethod::FindConversion(std::vector<std::shared_ptr<TSParameter>>& params, TSClass* ret_class)
{
	for (const std::shared_ptr<TSMethod>& i : methods)
		if (i->HasParams(params) && i->GetRetClass() == ret_class)
			return i.get();
	return NULL;
}

bool TSOverloadedMethod::ParamsExists(TSMethod* use_method)
{
	return FindParams(use_method->param) != NULL;
}

TSMethod* TSOverloadedMethod::FindParams(TSMethod* use_method)
{
	return FindParams(use_method->param);
}

void TSOverloadedMethod::GetMethods(std::vector<TSMethod*> &result)
{
	for (const std::shared_ptr<TSMethod>& i : methods)
		result.push_back(i.get());
}

void TSOverloadedMethod::CheckForErrors(bool is_conversion)
{
	for (const std::shared_ptr<TSMethod>& i : methods)
	{
		i->CheckForErrors();
		if (is_conversion)
		{
			if (FindConversion(i->param, i->GetRetClass()) != &(*i))
				i->GetSyntax()->Error("Метод с такими параметрами уже существует!");
		}
		else
		{
			if (FindParams(i->param) != &(*i))
				i->GetSyntax()->Error("Метод с такими параметрами уже существует!");
		}
	}
}