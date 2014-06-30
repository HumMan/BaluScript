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
	std::vector<std::unique_ptr<TMethod>> methods;
	TOverloadedMethod(){}
	TOverloadedMethod(TNameId use_name) :name(use_name){}
	TNameId GetName()const;
};	