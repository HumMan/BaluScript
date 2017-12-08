#pragma once

namespace SyntaxApi
{
	class IFor: public virtual IStatement
	{
	public:
		virtual IExpression* GetCompare()const = 0;
		virtual IStatements* GetIncrement()const = 0;
		virtual IStatements* GetStatements()const=0;
	};
}