#include "OverloadedMethod.h"

#include "Method.h"
#include "Statements.h"

using namespace Lexer;
using namespace SyntaxInternal;

TNameId TOverloadedMethod::GetName()const{
	return name;
}

TOverloadedMethod::TOverloadedMethod()
{

}
TOverloadedMethod::TOverloadedMethod(Lexer::TNameId use_name)
	:name(use_name)
{

}
void TOverloadedMethod::AddMethod(TMethod* method)
{
	methods.emplace_back(method);
}
void TOverloadedMethod::GetMethods(std::vector<TMethod*> &result)
{
	result.empty();
	for (auto &v : methods)
		result.push_back(v.get());
}
void TOverloadedMethod::GetMethods(std::vector<SyntaxApi::IMethod*> &result)
{
	result.empty();
	for (auto &v : methods)
		result.push_back(v.get());
}