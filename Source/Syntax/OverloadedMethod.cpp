#include "OverloadedMethod.h"

#include "ClassField.h"
#include "Method.h"
#include "Statements.h"

void TOverloadedMethod::InitOwner(TClass* use_owner)
{
	for (const std::shared_ptr<TMethod>& method : methods)
		method->InitOwner(use_owner);
}
TNameId TOverloadedMethod::GetName()const{
	return name;
}
TMethod* TOverloadedMethod::FindParams(std::vector<std::unique_ptr<TParameter>>& params)
{
	for (const std::shared_ptr<TMethod>& method : methods)
		if (method->HasParams(params))
			return method.get();
	return NULL;
}
TMethod* TOverloadedMethod::FindConversion(std::vector<std::unique_ptr<TParameter>>& params, TClass* ret_class)
{
	for (const std::shared_ptr<TMethod>& i : methods)
		if (i->HasParams(params) && i->GetRetClass() == ret_class)
			return i.get();
	return NULL;
}
bool TOverloadedMethod::ParamsExists(TMethod* use_method)
{
	return FindParams(use_method->param) != NULL;
}
TMethod* TOverloadedMethod::FindParams(TMethod* use_method)
{
	return FindParams(use_method->param);
}
void TOverloadedMethod::Declare()
{
	for (const std::shared_ptr<TMethod>& i : methods)
		i->Declare();
}
void TOverloadedMethod::Build(TNotOptimizedProgram &program)
{
	for (const std::shared_ptr<TMethod>& i : methods)
		i->Build(program);
}
void TOverloadedMethod::GetMethods(std::vector<TMethod*> &result)
{
	for (const std::shared_ptr<TMethod>& i : methods)
		result.push_back(i.get());
}
void TOverloadedMethod::CheckForErrors(bool is_conversion)
{
	for (const std::shared_ptr<TMethod>& i : methods)
	{
		i->CheckForErrors();
		if (is_conversion)
		{
			if (FindConversion(i->param, i->GetRetClass()) != &(*i))
				i->Error("Метод с такими параметрами уже существует!");
		}
		else
		{
			if (FindParams(i->param) != &(*i))
				i->Error("Метод с такими параметрами уже существует!");
		}
	}
}