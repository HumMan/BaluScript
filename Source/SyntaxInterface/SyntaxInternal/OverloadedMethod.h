#pragma once

#include "../SyntaxTreeApi.h"

namespace SyntaxInternal
{
	class TMethod;
	class TClass;

	class TOverloadedMethod: public SyntaxApi::IOverloadedMethod
	{
	private:
		Lexer::TNameId name;
		std::vector<std::unique_ptr<TMethod>> methods;
	public:		
		TOverloadedMethod();
		TOverloadedMethod(Lexer::TNameId use_name);
		void AddMethod(TMethod* method);
		void GetMethods(std::vector<TMethod*> &result);
		void GetMethods(std::vector<SyntaxApi::IMethod*> &result);
		Lexer::TNameId GetName()const;
	};
}