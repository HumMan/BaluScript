#include "Return.h"

using namespace Lexer;
using namespace SyntaxInternal;

void TReturn::AnalyzeSyntax(Lexer::ILexer* source) {
	InitPos(source);
	source->GetToken(TResWord::Return);
	if (!source->Test(TTokenType::Semicolon))
		result->AnalyzeSyntax(source);
}

void TReturn::Accept(SyntaxApi::IStatementVisitor* visitor)
{
	visitor->Visit(this);
}

TExpression* TReturn::GetResult()const
{
	return result.get();
}