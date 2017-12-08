#pragma once

namespace SyntaxApi
{
	class IIf: public virtual IStatement
	{
	public:
		virtual IExpression* GetBoolExpr()const=0;
		virtual IStatements* GetStatements()const=0;
		virtual IStatements* GetElseStatements()const=0;
	};
}