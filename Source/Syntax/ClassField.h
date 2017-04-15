#pragma once

#include "../SyntaxTree/SyntaxTreeApi.h"

#include "Accessible.h"

namespace SyntaxInternal
{
	class TClass;
	class TType;

	class TClassField :public TAccessible, public Lexer::TTokenPos, public TMultifield, public SyntaxApi::IClassField
	{
		TClass* owner;
		Lexer::TNameId name;
		std::shared_ptr<TType> type;
		bool is_static;
		bool read_only;
	public:
		SyntaxApi::IType* GetType()const;
		bool IsReadOnly()const;
		void SetReadOnly(bool use_read_only);
		TClassField(TClass* use_owner);
		void AnalyzeSyntax(Lexer::ILexer* source);
		TClass* GetOwner()const;
		Lexer::TNameId GetName()const;
		bool IsStatic()const;
	};
}