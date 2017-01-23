#pragma once

#include "../lexer.h"
#include "Class.h"

class TMethod;
class TType;
class TClass;

///<summary>Класс описывает параметр объявления метода</summary>
class TParameter :public Lexer::TTokenPos
{
	std::unique_ptr<TType> type;
	bool is_ref;
	Lexer::TNameId name;
	TClass* owner;
	TMethod* method;
public:
	TParameter(TClass* use_owner, TMethod* use_method);
	void AnalyzeSyntax(Lexer::ILexer* source);
	Lexer::TNameId GetName()const;
	TType* GetType()const;
	bool IsRef()const;
};