#include "Parameter.h"

#include "Method.h"
#include "Statements.h"
#include "Type.h"

using namespace Lexer;
using namespace SyntaxInternal;

TParameter::TParameter(TClass* use_owner, TMethod* use_method)
	: type(new TType(use_owner)), is_ref(false)
	, owner(use_owner), method(use_method)
{
}

void TParameter::AnalyzeSyntax(Lexer::ILexer* source) 
{
	InitPos(source);
	type->AnalyzeSyntax(source);
	is_ref = source->TestAndGet(TTokenType::Ampersand);

	name = source->NameId();
	source->GetToken(TTokenType::Identifier);
}

Lexer::TNameId TParameter::GetName()const
{
	return name;
}
TType* TParameter::GetType()const
{
	return type.get();
}
bool TParameter::IsRef()const{
	return is_ref;
}