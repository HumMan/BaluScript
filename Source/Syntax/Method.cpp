
void TMethod::ParametersDecl(TLexer& source) {
	//—читываем все параметры метода и возвращаемое значение
	if (source.TestAndGet(TTokenType::Operator, TOperator::ParamsCall)) {
	} else if (source.Test(TTokenType::LParenth)) {
		source.GetToken(TTokenType::LParenth);
		while (source.Test(TTokenType::Identifier)) {
			TParameter* t = param.Push(new TParameter(owner, this));
			t->AnalyzeSyntax(source);
			if (source.Test(TTokenType::Comma))
				source.GetToken();
			else
				break;
		}
		source.GetToken(TTokenType::RParenth);
	}
	if (source.TestAndGet(TTokenType::Colon)) {
		ret_ref = source.TestAndGet(TTokenType::Ampersand);//TODO проверка ссылки после типа а то не пон¤тно почему ошибка
		ret.AnalyzeSyntax(source);
	}
}

void TMethod::AnalyzeSyntax(TLexer& source, bool realization) {
	InitPos(source);
	source.TestToken(TTokenType::ResWord);
	member_type = (TClassMember::Enum) source.Token();
	if (!IsIn(member_type, TClassMember::Func, TClassMember::Conversion))
		source.Error(
				"ќжидалось объ¤вление метода,конструктора,деструктора,оператора или приведени¤ типа!");
	source.GetToken();
	is_extern = source.TestAndGet(TTokenType::ResWord, TResWord::Extern);
	is_static = source.TestAndGet(TTokenType::ResWord, TResWord::Static);
	switch (member_type) {
	case TResWord::Func:
	case TResWord::Constr:
	case TResWord::Destr:
	case TResWord::Operator:
	case TResWord::Conversion:
		break;
	default:
		source.Error(
				"ќжидалось объ¤вление метода,конструктора,деструктора,оператора или приведени¤ типа!");
	}
	//
	if (!realization) {
		if (source.NameId() != owner->GetName())
			Error("ќжидалось им¤ базового класса!");
		source.GetToken();
		while (source.Test(TTokenType::Dot)) {
			source.GetToken(TTokenType::Dot);
			if (!source.Test(TTokenType::Identifier))
				break;
			TClass* t = owner->GetNested(source.NameId());
			if (t != NULL) {
				source.GetToken();
				owner = t;
			} else
				break;
		}
	}
	//
	if (member_type == TClassMember::Func) {
		source.TestToken(TTokenType::Identifier);
		method_name = source.NameId();
		source.GetToken();
	} else if (member_type == TClassMember::Operator) {
		source.TestToken(TTokenType::Operator);
		operator_type = (TOperator::Enum) source.Token();
		source.GetToken();
	}
	{
		ParametersDecl(source);
		if (realization) {
			switch (member_type) {
			case TClassMember::Func:
				owner->AddMethod(this, method_name);
				break;
			case TClassMember::Constr:
				owner->AddConstr(this);
				break;
			case TClassMember::Destr:
				owner->AddDestr(this);
				break;
			case TClassMember::Operator:
				if (operator_type == TOperator::Minus || operator_type
						== TOperator::UnaryMinus) {
					if (GetParamsHigh() == 0)
						operator_type = TOperator::UnaryMinus;
					else
						operator_type = TOperator::Minus;
				}
				owner->AddOperator(operator_type, this);
				break;
			case TClassMember::Conversion:
				owner->AddConversion(this);
				break;
			default:
			assert(false);
			}
		}
	}
	if (is_extern && realization) {
		if (!source.Test(TTokenType::Semicolon))
			source.Error("?л¤ внешнего метода тело должно отсутствовать!");
		source.GetToken(TTokenType::Semicolon);
	} else if (realization) {
		one_instruction = !source.Test(TTokenType::LBrace);
		statements.AnalyzeSyntax(source);
	}
}
