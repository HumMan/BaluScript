#pragma once

namespace SyntaxApi
{
	class IReturn: public virtual IStatement
	{
	public:
		virtual IExpression* GetResult()const=0;
	};
}