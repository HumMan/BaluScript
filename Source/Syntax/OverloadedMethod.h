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
	Lexer::TNameId name;
public:
	std::vector<std::unique_ptr<TMethod>> methods;
	TOverloadedMethod(){}
	TOverloadedMethod(Lexer::TNameId use_name) :name(use_name){}
	void GetMethods(std::vector<TMethod*> &result);
	Lexer::TNameId GetName()const;
};	