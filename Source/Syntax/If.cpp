void TIf::AnalyzeSyntax(TLexer& source) {
	InitPos(source);
	source.GetToken(TTokenType::ResWord, TResWord::If);
	source.GetToken(TTokenType::LParenth);

	bool_expr.AnalyzeSyntax(source);
	source.GetToken(TTokenType::RParenth);
	statements.AnalyzeSyntax(source);
	if (source.Test(TTokenType::ResWord, TResWord::Else)) {
		source.GetToken(TTokenType::ResWord, TResWord::Else);
		else_statements.AnalyzeSyntax(source);
	}
}