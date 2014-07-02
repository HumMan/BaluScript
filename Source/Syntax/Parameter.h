#pragma once

#include "../lexer.h"
#include "Class.h"

class TMethod;

///<summary>Класс описывает параметр объявления метода</summary>
class TParameter :public TTokenPos
{
	friend class TSMethod;
	TType type;
	bool is_ref;
	TNameId name;
	TClass* owner;
	TMethod* method;
public:
	TParameter(TClass* use_owner, TMethod* use_method) 
		: type(use_owner), is_ref(false)
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