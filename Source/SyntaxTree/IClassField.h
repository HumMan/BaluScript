#pragma once

namespace SyntaxApi
{
	class IMultiField
	{
	public:
		virtual bool HasSizeMultiplierId()=0;
		virtual bool HasSizeMultiplier()=0;
		virtual int GetSizeMultiplier()=0;
		virtual Lexer::TNameId GetFactorId()=0;
	};

	class IClassField : public virtual IAccessible, public virtual Lexer::ITokenPos, public virtual IMultiField
	{
	public:
		virtual Lexer::TNameId GetName()const=0;
		virtual bool IsStatic()const=0;
		virtual bool IsReadOnly()const=0;
		virtual IType* GetType()const=0;
	};
}