#pragma once

#include "../lexer.h"
#include "Variable.h"
#include "Class.h"

class TMethod;

///<summary>Класс описывает параметр объявления метода</summary>
class TParameter :public TVariable, public TTokenPos
{
	TType type;
	bool is_ref;
	TNameId name;
	TClass* owner;
	TMethod* method;
public:
	TParameter(TClass* use_owner, TMethod* use_method) :TVariable(TVariableType::Parameter)
		, type(use_owner), is_ref(false)
		, owner(use_owner), method(use_method)
	{
	}
	void AnalyzeSyntax(TLexer& source);
	TNameId GetName()const{
		return name;
	}
	TNameId GetClassName()const;

	bool IsRef()const{
		return is_ref;
	}
};