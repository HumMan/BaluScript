#include "OverloadedMethod.h"

#include "Method.h"

void TOverloadedMethod::InitOwner(TClass* use_owner)
{
	//typedef std::list<TMethod>::iterator it;
	for (auto i = methods.begin(); i != methods.end(); i++)
		i->InitOwner(use_owner);
}
TNameId TOverloadedMethod::GetName()const{
	return name;
}
TMethod* TOverloadedMethod::FindParams(TVectorList<TParameter>& params)
{
	for (auto i = methods.begin(); i != methods.end(); i++)
		if (i->HasParams(params))
			return &(*i);
	return NULL;
}
TMethod* TOverloadedMethod::FindConversion(TVectorList<TParameter>& params, TClass* ret_class)
{
	for (auto i = methods.begin(); i != methods.end(); i++)
		if (i->HasParams(params) && i->GetRetClass() == ret_class)return &(*i);
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
	for (auto i = methods.begin(); i != methods.end(); i++)
		i->Declare();
}
void TOverloadedMethod::Build(TNotOptimizedProgram &program)
{
	for (auto i = methods.begin(); i != methods.end(); i++)i->Build(program);
}
void TOverloadedMethod::GetMethods(TVector<TMethod*> &result)
{
	for (auto i = methods.begin(); i != methods.end(); i++)
		result.Push(&(*i));
}
void TOverloadedMethod::CheckForErrors(bool is_conversion)
{
	for (auto i = methods.begin(); i != methods.end(); i++)
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