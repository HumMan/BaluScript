#pragma once

#include "../lexer.h"
#include "Class.h"

class TMethod;
class TType;
class TClass;

///<summary>Класс описывает параметр объявления метода</summary>
class TParameter :public Lexer::TTokenPos
{
	friend class TSMethod;
	std::unique_ptr<TType> type;
	bool is_ref;
	Lexer::TNameId name;
	TClass* owner;
	TMethod* method;
public:
	TParameter(TClass* use_owner, TMethod* use_method);
	void AnalyzeSyntax(Lexer::ILexer* source);
	Lexer::TNameId GetName()const{
		return name;
	}
	Lexer::TNameId GetClassName()const;

	bool IsRef()const{
		return is_ref;
	}
};