#pragma once

#include "../lexer.h"
#include <list>
#include <vector>

#include "../notOptimizedProgram.h"

class TParameter;
class TMethod;
class TClass;

class TOverloadedMethod
{
private:
	TNameId name;
public:
	std::vector<std::shared_ptr<TMethod>> methods;
	TOverloadedMethod(){}
	TOverloadedMethod(TNameId use_name):name(use_name){}
	void InitOwner(TClass* use_owner);
	TNameId GetName()const;
	TMethod* FindParams(std::vector<std::unique_ptr<TParameter>>& params);
	TMethod* FindConversion(std::vector<std::unique_ptr<TParameter>>& params, TClass* ret_class);
	bool ParamsExists(TMethod* use_method);
	TMethod* FindParams(TMethod* use_method);
	void Declare();
	void Build(TNotOptimizedProgram &program);
	void GetMethods(std::vector<TMethod*> &result);
	void CheckForErrors(bool is_conversion = false);
};	