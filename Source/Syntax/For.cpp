#include "For.h"

void TFor::AnalyzeSyntax(TLexer& source) {
	InitPos(source);

	compare.AnalyzeSyntax(source);
	source.GetToken(TTokenType::Semicolon);
	increment.AnalyzeStatement(source, false);
	source.GetToken(TTokenType::RParenth);
	statements.AnalyzeSyntax(source);
}
