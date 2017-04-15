#pragma once

namespace SyntaxApi
{
	class IParameter : public virtual Lexer::ITokenPos
	{
	public:
		virtual Lexer::TNameId GetName()const=0;
		virtual IType* GetType()const=0;
		virtual bool IsRef()const=0;
	};
}