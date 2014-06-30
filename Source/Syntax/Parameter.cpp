#include "Parameter.h"

#include "Method.h"
#include "Statements.h"

void TParameter::AnalyzeSyntax(TLexer& source) 
{
	InitPos(source);
	type.AnalyzeSyntax(source);
	is_ref = source.TestAndGet(TTokenType::Ampersand);

	name = source.NameId();
	source.GetToken(TTokenType::Identifier);
}