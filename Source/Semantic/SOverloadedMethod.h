#pragma once

#include "../lexer.h"
#include <list>
#include <vector>
#include <memory>

class TSParameter;
class TSMethod;
class TSClass;

class TSOverloadedMethod
{
public:
	std::vector<std::shared_ptr<TSMethod>> methods;
	TSOverloadedMethod(){}
	TNameId GetName()const;
	TSMethod* FindParams(std::vector<std::shared_ptr<TSParameter>>& params);
	TSMethod* FindConversion(std::vector<std::shared_ptr<TSParameter>>& params, TSClass* ret_class);
	bool ParamsExists(TSMethod* use_method);
	TSMethod* FindParams(TSMethod* use_method);
	void GetMethods(std::vector<TSMethod*> &result);
	void CheckForErrors(bool is_conversion = false);
};	