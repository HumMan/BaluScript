
void TStatements::AnalyzeStatement(TLexer& source, bool end_semicolon) {
	switch (source.Type()) {
	case TTokenType::LBrace: {
		TStatements* s = new TStatements(owner, method, this,
				statement.GetHigh() + 1);
		Add(s);
		s->AnalyzeSyntax(source);
		return;
	}
	case TTokenType::ResWord: {
		switch (source.Token()) {
		case TResWord::Return: {
			TReturn* t = new TReturn(owner, method, this, statement.GetHigh()
					+ 1);
			Add(t);
			t->AnalyzeSyntax(source);
			if (!end_semicolon)
				source.Error("return можно использовать только как отдельную инструкцию!");
			source.GetToken(TTokenType::Semicolon);
			return;
		}
		case TResWord::If: {
			TIf* t = new TIf(owner, method, this, statement.GetHigh() + 1);
			Add(t);
			t->AnalyzeSyntax(source);
			return;
		}
		case TResWord::For: {
			source.GetToken(TTokenType::ResWord, TResWord::For);
			source.GetToken(TTokenType::LParenth);

			TStatements* for_stmt = new TStatements(owner, method, this,
					statement.GetHigh() + 1);
			Add(for_stmt);

			TFor* t =
					new TFor(owner, method, for_stmt, for_stmt->GetHigh() + 1);
			for_stmt->AnalyzeSyntax(source);
			for_stmt->Add(t);

			t->AnalyzeSyntax(source);
			return;
		}
		case TResWord::This: {
			TExpression* t = new TExpression(owner, method, this,
					statement.GetHigh() + 1);
			Add(t);
			t->AnalyzeSyntax(source);
			if (end_semicolon)
				source.GetToken(TTokenType::Semicolon);
			return;
		}
		case TResWord::Bytecode: {
			TBytecode* t = new TBytecode(owner, method, this,
					statement.GetHigh() + 1);
			Add(t);
			t->AnalyzeSyntax(source);
			if (end_semicolon)
				source.GetToken(TTokenType::Semicolon);
			return;
		}
		}
		source.Error("ќжидалс¤ return,if,for,bytecode или this!");
	}
	default:
		if (IsVarDecl(source)) {
			TLocalVar* t = new TLocalVar(owner, method, this,
					statement.GetHigh() + 1);
			Add(t);
			var_declarations.Push(TVarDecl(statement.GetHigh(), t));
			t->AnalyzeSyntax(source);
		} else {
			TExpression* t = new TExpression(owner, method, this,
					statement.GetHigh() + 1);
			Add(t);
			t->AnalyzeSyntax(source);
		}
		if (end_semicolon)
			source.GetToken(TTokenType::Semicolon);
		return;
	}
	assert(false);//во всех switch должен быть return 
}
void TStatements::AnalyzeSyntax(TLexer& source) {
	InitPos(source);
	if (source.Test(TTokenType::LBrace)) {
		source.GetToken(TTokenType::LBrace);
		while (!source.TestAndGet(TTokenType::RBrace))
			AnalyzeStatement(source, true);
	} else
		AnalyzeStatement(source, true);
}
