#pragma once

namespace SyntaxApi
{
	class IOverloadedMethod
	{
	public:
		virtual void GetMethods(std::vector<IMethod*> &result)=0;
		virtual Lexer::TNameId GetName()const=0;
	};
}