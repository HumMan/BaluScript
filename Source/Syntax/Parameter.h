#pragma once

#include "../SyntaxTree/SyntaxTreeApi.h"

#include "Class.h"
#include "Type.h"

namespace SyntaxInternal
{
	class TMethod;
	class TType;
	class TClass;

	///<summary>Класс описывает параметр объявления метода</summary>
	class TParameter :public Lexer::TTokenPos, public SyntaxApi::IParameter
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
}