#pragma once

namespace SyntaxApi
{
	enum class TClassMember
	{
		Func = (short)Lexer::TResWord::Func,
		DefaultConstr,
		CopyConstr,
		MoveConstr,
		Destr,
		Operator,
		Conversion
	};

	class IMethod: public virtual Lexer::ITokenPos, public virtual IAccessible
	{
	public:
		virtual bool HasReturn()const = 0;
		virtual Lexer::TNameId GetName()const=0;
		virtual bool IsReturnRef()const = 0;
		virtual IType* GetRetType()const = 0;
		virtual TClassMember GetMemberType()const=0;
		virtual IStatements* GetStatements()const=0;
		virtual IParameter* GetParam(size_t use_id)const = 0;
		virtual size_t GetParamsCount()const = 0;
		virtual bool IsStatic()const = 0;
		virtual bool IsExternal()const = 0;
		virtual Lexer::TOperator GetOperatorType()const = 0;
		virtual IClass* GetOwner()const=0;
	};
}