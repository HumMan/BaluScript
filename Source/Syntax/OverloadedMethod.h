#pragma once

#include "../lexer.h"
#include <list>
#include <vector>
#include <memory>

class TMethod;
class TClass;

class TOverloadedMethod
{
private:
	TNameId name;
public:
	std::vector<std::shared_ptr<TMethod>> methods;
	TOverloadedMethod(){}
	TOverloadedMethod(TNameId use_name) :name(use_name){}
	void GetMethods(std::vector<TMethod*> &result);
	TNameId GetName()const;
};	