#pragma once

namespace SyntaxApi
{
	class IWhile : public virtual SyntaxApi::IStatement
	{
	public:
		virtual IExpression* GetCompare()const = 0;
		virtual IStatements* GetStatements()const = 0;
	};
}