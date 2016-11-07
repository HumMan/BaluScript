#include "Return.h"

using namespace Lexer;

void TReturn::AnalyzeSyntax(Lexer::ILexer* source) {
	InitPos(source);
	source->GetToken(TTokenType::ResWord, TResWord::Return);
	if (!source->Test(TTokenType::Semicolon))
		result.AnalyzeSyntax(source);
}

void TReturn::Accept(TStatementVisitor* visitor)
{
	visitor->Visit(this);
}