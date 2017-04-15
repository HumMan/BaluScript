#pragma once

namespace SyntaxApi
{
	class ILocalVar : public virtual IStatement
	{
	public:
		virtual IExpression* GetAssignExpr()const = 0;
		virtual std::vector<IExpression*> GetParams()const = 0;
		virtual Lexer::TNameId GetName()const=0;
		virtual bool IsStatic() = 0;
		virtual IType* GetVarType() = 0;
	};
}