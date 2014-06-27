#include "Return.h"

void TReturn::AnalyzeSyntax(TLexer& source) {
	InitPos(source);
	source.GetToken(TTokenType::ResWord, TResWord::Return);
	if (!source.Test(TTokenType::Semicolon))
		result.AnalyzeSyntax(source);
}
