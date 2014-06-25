#include "While.h"

void TWhile::AnalyzeSyntax(TLexer& source) {
	InitPos(source);
	source.GetToken(TTokenType::ResWord, TResWord::While);
	source.GetToken(TTokenType::LParenth);

	compare.AnalyzeSyntax(source);
	source.GetToken(TTokenType::RParenth);
	statements.AnalyzeSyntax(source);
}