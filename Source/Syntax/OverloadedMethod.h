#pragma once

#include "../lexer.h"
#include <list>

#include "../notOptimizedProgram.h"

class TParameter;
class TMethod;
class TClass;

class TOverloadedMethod
{
private:
	TNameId name;
public:
	std::list<TMethod> methods;
	TOverloadedMethod(){}
	TOverloadedMethod(TNameId use_name):name(use_name){}
	void InitOwner(TClass* use_owner);
	TNameId GetName()const;
	TMethod* FindParams(TVectorList<TParameter>& params);
	TMethod* FindConversion(TVectorList<TParameter>& params, TClass* ret_class);
	bool ParamsExists(TMethod* use_method);
	TMethod* FindParams(TMethod* use_method);
	void Declare();
	void Build(TNotOptimizedProgram &program);
	void GetMethods(TVector<TMethod*> &result);
	void CheckForErrors(bool is_conversion = false);
};	