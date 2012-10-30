#include "baluScript.h"

TType::TType(TNameId use_class_name, TClass *use_owner) :
	owner(use_owner) {
	class_pointer = NULL;
	class_name.name = use_class_name;
	class_name.member = NULL;
}

TType::TType(TClass *use_owner) :
	owner(use_owner) {
	class_pointer = NULL;
}

void TType::AnalyzeSyntax(TClassName* use_class_name, TLexer& source) {
	source.TestToken(TTokenType::Identifier);
	use_class_name->name = source.NameId();
	use_class_name->member = NULL;
	source.GetToken(TTokenType::Identifier);
	if (source.Test(TTokenType::Operator, TOperator::Less)) {
		source.GetToken();
		while (!source.TestAndGet(TTokenType::Operator, TOperator::Greater)) {
			TClassName* template_param = new TClassName();
			use_class_name->template_params.Push(template_param);
			AnalyzeSyntax(template_param, source);
			if (!source.Test(TTokenType::Operator, TOperator::Greater))
				source.GetToken(TTokenType::Comma);
		}
	}
	if (source.Test(TTokenType::Dot)) {
		source.GetToken();
		use_class_name->member = new TClassName();
		AnalyzeSyntax(use_class_name->member.GetPointer(), source);
	}
}

void TType::AnalyzeSyntax(TLexer& source) {
	InitPos(source);
	AnalyzeSyntax(&class_name, source);
	if (source.Test(TTokenType::LBracket) || source.Test(TTokenType::Operator,
			TOperator::GetArrayElement)) {
		while (true) {
			if (source.TestAndGet(TTokenType::LBracket)) {
				if (source.Test(TTokenType::Value, TValue::Int)) {
					dim.Push(source.Int());
					source.GetToken();
				}
				source.GetToken(TTokenType::RBracket);
			} else if (source.TestAndGet(TTokenType::Operator,
					TOperator::GetArrayElement)) {
				dim.Push(0);
			} else
				break;
		}
	}
}

void TStatements::AddVar(TLocalVar* use_var) {
	statement.Push((TStatement*) use_var);
	use_var->SetStmtId(statement.GetHigh());
	var_declarations.Push(TVarDecl(statement.GetHigh(), use_var));
}

void TLocalVar::AnalyzeSyntax(TLexer& source) {
	InitPos(source);
	type.AnalyzeSyntax(source);
	is_static = source.TestAndGet(TTokenType::ResWord, TResWord::Static);
	assert(parent->GetType()==TStatementType::Statements);
	TStatements* statements = (TStatements*) parent;
	TLocalVar* curr_var = this;
	do {
		if (curr_var != this) {
			curr_var->type = type;
			curr_var->is_static = is_static;
			*(TTokenPos*) curr_var = *(TTokenPos*) this;
		}
		curr_var->name = source.NameId();
		int identifier_token = source.GetCurrentToken();
		source.SetCurrentToken(identifier_token + 1);
		bool is_next_assign = source.Test(TTokenType::Operator,
				TOperator::Assign);
		source.SetCurrentToken(identifier_token);
		if (is_next_assign) {
			curr_var->assign_expr = new TExpression(owner, method, parent,
					curr_var->stmt_id);
			curr_var->assign_expr->AnalyzeSyntax(source);
		} else {
			source.GetToken();
			if (source.TestAndGet(TTokenType::LParenth)) {
				while (!source.Test(TTokenType::LParenth)) {
					TExpression* expr = new TExpression(owner, method, parent,
							curr_var->stmt_id);
					expr->AnalyzeSyntax(source);
					curr_var->params.Push(expr);
					if (!source.TestAndGet(TTokenType::Comma))
						break;
				}
				source.GetToken(TTokenType::RParenth);
			}
		}
		if (source.Test(TTokenType::Comma)) {
			curr_var = new TLocalVar(owner, method, statements, -1);
			statements->AddVar(curr_var);
		}
	} while (source.TestAndGet(TTokenType::Comma));
}

void TClass::AnalyzeSyntax(TLexer& source) {
	InitPos(source);

	if(source.TestAndGet(TTokenType::ResWord,TResWord::Enum))
	{
		size=1;
		is_enum=true;
		source.TestToken(TTokenType::Identifier);
		name=source.NameId();
		source.GetToken();
		source.GetToken(TTokenType::LBrace);
		do
		{
			source.Test(TTokenType::Identifier);
			for(int i=0;i<=enums.GetHigh();i++)
				if(enums[i]==source.NameId())
					source.Error("ѕеречисление с таким имененм уже существует!");
			enums.Push(source.NameId());
			source.GetToken();
		}while(source.TestAndGet(TTokenType::Comma));
		source.GetToken(TTokenType::RBrace);
		return;
	}

	source.GetToken(TTokenType::ResWord, TResWord::Class);
	is_sealed = source.TestAndGet(TTokenType::ResWord, TResWord::Sealed);
	source.TestToken(TTokenType::Identifier);
	name = source.NameId();
	source.GetToken();
	if (source.TestAndGet(TTokenType::Operator, TOperator::Less)) {
		is_template = true;
		do {
			source.TestToken(TTokenType::Identifier);
			TTemplateParam p;
			p.class_pointer = NULL;
			p.name = source.NameId();
			template_params.Push(p);
			source.GetToken();
			if (!source.TestAndGet(TTokenType::Comma))
				break;
		} while (true);
		source.GetToken(TTokenType::Operator, TOperator::Greater);
	}
	if (source.TestAndGet(TTokenType::Colon)) {
		source.TestToken(TTokenType::Identifier);
		parent.AnalyzeSyntax(source);
	}
	source.GetToken(TTokenType::LBrace);

	bool readonly = false;
	TTypeOfAccess::Enum access = TTypeOfAccess::Public;

	while (true) {
		bool end_while = false;
		AccessDecl(source, readonly, access);
		if (source.Type() == TTokenType::ResWord)
			switch (source.Token()) {
			case TResWord::Class: {
				TClass* nested = new TClass(this, templates);
				AddNested(nested);
				nested->AnalyzeSyntax(source);
			}
				break;
			case TResWord::Enum: {
				TClass* enumeraiton= new TClass(this,templates);
				AddNested(enumeraiton);
				enumeraiton->AnalyzeSyntax(source);
			}
				break;
			case TResWord::Func:
			case TResWord::Constr:
			case TResWord::Destr:
			case TResWord::Operator:
			case TResWord::Conversion: {
				TMethod* func = new TMethod(this);
				func->SetAccess(access);
				func->AnalyzeSyntax(source);
			}
				break;
			default:
				end_while = true;
			}
		else if (source.Type() == TTokenType::Identifier) {
			TClassField* temp_field = new TClassField(this);
			field.Push(temp_field);
			temp_field->SetAccess(access);
			temp_field->SetReadOnly(readonly);
			temp_field->AnalyzeSyntax(source);
		} else
			break;
		if (end_while)
			break;
	}
	source.GetToken(TTokenType::RBrace);
	if (owner == NULL)
		source.GetToken(TTokenType::Done);//в файле должен содержатьс€ только один основной класс
}

TExpression::TOperation* TExpression::ParamsCall(TLexer& source,
		TExpression::TOperation* curr_operation) {
	bool use_brackets = source.Test(TTokenType::LBracket) || source.Test(
			TTokenType::Operator, TOperator::GetArrayElement);
	TExpression::TCallParamsOp* params_call = new TCallParamsOp(curr_operation,
			use_brackets);
	params_call->InitPos(source);

	if (use_brackets) {
		if (!source.TestAndGet(TTokenType::Operator, TOperator::GetArrayElement))
			source.GetToken(TTokenType::LBracket);
		else
			return params_call;
	} else {
		if (!source.TestAndGet(TTokenType::Operator, TOperator::ParamsCall))
			source.GetToken(TTokenType::LParenth);
		else
			return params_call;
	}
	while (!source.Test(use_brackets ? (TTokenType::RBracket)
			: (TTokenType::RParenth))) {
		params_call->AddParam(Expr(source, 0));
		if (!source.TestAndGet(TTokenType::Comma))
			break;
	}
	source.GetToken(use_brackets ? (TTokenType::RBracket)
			: (TTokenType::RParenth));
	return params_call;
}

TExpression::TOperation* TExpression::SpecialPostfix(TLexer& source,
		TExpression::TOperation* curr_operation) {
	if (source.TestAndGet(TTokenType::Dot)) {
		source.TestToken(TTokenType::Identifier);
		curr_operation = new TGetMemberOp(curr_operation, source.NameId());
		curr_operation->InitPos(source);
		source.GetToken();
		return curr_operation;
	} else if (source.Test(TTokenType::LBracket) || source.Test(
			TTokenType::LParenth) || source.Test(TTokenType::Operator,
			TOperator::ParamsCall) || source.Test(TTokenType::Operator,
			TOperator::GetArrayElement)) {
		return ParamsCall(source, curr_operation);
	} else {
		return curr_operation;
	}
}

TExpression::TOperation* TExpression::Factor(TLexer& source) {
	TOperation* curr_operation = NULL;
	TTokenPos token_pos;
	token_pos.InitPos(source);
	if (source.Test(TTokenType::Value))
	//—интаксис: Value
	{
		switch (source.Token()) {
		case TValue::Int:
			curr_operation = new TIntValue(source.Int(), source.GetIdFromName(
					"int"), owner);
			break;
		case TValue::Float:
			curr_operation = new TFloatValue(source.Float(),
					source.GetIdFromName("float"), owner);
			break;
		case TValue::Bool:
			curr_operation = new TBoolValue(source.Bool(),
					source.GetIdFromName("bool"), owner);
			break;
		case TValue::String:
			curr_operation = new TStringValue(source.String(),
					source.GetIdFromName("string"), owner);
			break;
		case TValue::Char:
			curr_operation = new TCharValue(source.Char(),
					source.GetIdFromName("char"), owner);
			break;
		default:
		assert(false);
		}
		source.GetToken();
	} else {
		if (source.TestAndGet(TTokenType::ResWord, TResWord::This)) {
			curr_operation = new TThis();
			*(TTokenPos*) curr_operation = token_pos;
		} else if (source.Test(TTokenType::Identifier))
		//—интаксис: Identifier
		{
			curr_operation = new TId(source.NameId());
			*(TTokenPos*) curr_operation = token_pos;
			source.GetToken();
		} else if (source.TestAndGet(TTokenType::LParenth))
		//—интаксис: Expr
		{
			curr_operation = Expr(source, 0);
			source.GetToken(TTokenType::RParenth);
		}
		if (curr_operation == NULL)
			source.Error("ќшибка в выражении!");
		//—интаксис: SpecialPostfixOp*
		do {
			curr_operation = SpecialPostfix(source, curr_operation);
		} while (source.Test(TTokenType::LParenth) || source.Test(
				TTokenType::Operator, TOperator::ParamsCall) || source.Test(
				TTokenType::LBracket) || source.Test(TTokenType::Dot));
	}
	*(TTokenPos*) curr_operation = token_pos;
	return curr_operation;
}

const int operators_priority[TOperator::End] = {
/*OP_PLUS*/6,
/*TOperator::Minus*/6,
/*OP_MUL*/7,
/*OP_DIV*/7,
/*OP_PERCENT*/7,

/*OP_AND*/2,
/*OP_OR*/1,
/*OP_NOT*/8,

/*OP_ASSIGN*/0,
/*OP_PLUS_A*/0,
/*OP_MINUS_A*/0,
/*OP_MUL_A*/0,
/*OP_DIV_A*/0,
/*OP_PERCENT_A*/0,
/*OP_AND_A*/0,
/*OP_OR_A*/0,

/*OP_LESS*/5,
/*OP_LESS_EQ*/5,
/*OP_EQUAL*/4,
/*OP_NOT_EQ*/3,
/*OP_GREATER*/5,
/*OP_GREATER_EQ*/5,

/*ParamsCall*/-1,
/*GetArrayElement*/-1,

/*TOperator::UnaryMinus*/8

};

TExpression::TOperation* TExpression::Expr(TLexer& source, int curr_prior_level) {

	const int operators_priority_max = 8;

	//TODO
	//dyn_test2=dyn_test1; если dyn_test1 не объ€влен то в ошибке неправильно указываетс€ символ

	TOperation *left;
	TOperator::Enum curr_op, curr_unary_op;
	if (curr_prior_level > operators_priority_max) {
		return Factor(source);
	}
	if (source.Test(TTokenType::Operator) && 
		(source.Token()	== TOperator::UnaryMinus || source.Token() == TOperator::Not )
					&& operators_priority[source.Token()] == curr_prior_level) {
		TTokenPos temp;
		temp.InitPos(source);
		if (source.Token() == TOperator::Minus)
			curr_unary_op = TOperator::UnaryMinus;
		else
			curr_unary_op = TOperator::Enum(source.Token());
		source.GetToken();
		TOperation* t = new TUnaryOp(Expr(source, curr_prior_level),
				curr_unary_op);
		*(TTokenPos*) t = temp;
		return t;
	} else
		left = Expr(source, curr_prior_level + 1);
	while (source.Test(TTokenType::Operator)
			&& operators_priority[source.Token()] == curr_prior_level) {
		TTokenPos temp;
		temp.InitPos(source);
		curr_op = TOperator::Enum(source.Token());
		source.GetToken();
		left = new TBinOp(left, Expr(source, curr_prior_level + 1), curr_op);
		*(TTokenPos*) left = temp;
	}
	return left;
}

void TExpression::AnalyzeSyntax(TLexer& source) {
	InitPos(source);
	first_op = Expr(source, 0);
	//BuildPostfix();
}

void TExpression::BuildPostfix() {
	while (true) {
		TPostfixOp temp;
		TTokenType::Enum token = source->Type();
		switch (token) {
		case TTokenType::Identifier:
			temp.type = TPostfixOp::OP_ID;
			temp.id = source->NameId();
			out.Push(temp);
			source->GetToken();
			break;
		case TTokenType::Value:
			switch (source->Token()) {
			case TValue::Int:
				temp.type = TPostfixOp::OP_INT;
				temp.int_val = source->Int();
				break;
			case TValue::Float:
				temp.type = TPostfixOp::OP_FLOAT;
				temp.float_val = source->Float();
				break;
			case TValue::Bool:
				temp.type = TPostfixOp::OP_BOOL;
				temp.int_val = source->Bool();
				break;
			}
			out.Push(temp);
			source->GetToken();
			break;
		case TTokenType::Operator:
			temp.type = TPostfixOp::OP_OPERATOR;
			temp.operator_type = source->Token();
			stack.Push(temp);
			{
				int t = stack.GetHigh();
				while (t > 0 && stack[t - 1].type == TPostfixOp::OP_OPERATOR
						&& operators_priority[stack[t - 1].operator_type]
								>= operators_priority[stack[t].operator_type])
					stack.Swap(t - 1, t);//TODO достаточно помен€ть только тип оператора
			}
			source->GetToken();
			break;
		default:
			while (stack.GetHigh() > -1)
				out.Push(stack.GetPop());
			return;
		}
	}
}

void TBytecode::AnalyzeSyntax(TLexer& source) {
	InitPos(source);
	source.GetToken(TTokenType::ResWord, TResWord::Bytecode);
	source.GetToken(TTokenType::LBrace);
	while (source.Test(TTokenType::Bytecode)) {
		int type = source.Token();
		source.GetToken();
		int params_count = 0;
		int params[4];
		TBytecodeOp _op;
		while (source.Test(TTokenType::Value, TValue::Bool) || source.Test(
				TTokenType::Value, TValue::Int) || source.Test(
				TTokenType::Value, TValue::Float) || source.Test(
				TTokenType::Identifier)) {
			if (source.Test(TTokenType::Identifier)) {
				if (source.NameId() != source.GetIdFromName(
						"CreateArrayElementClassId"))
					source.Error("Ќеизвестный идентификатор!");
				source.GetToken();
				source.GetToken(TTokenType::LParenth);
				source.TestToken(TTokenType::Identifier);
				//if(params_count<2||params_count>4)
				//	source.Error("ќшибка в выражении");
				//TODO разгрести это+ сделать нормальные константные выражени€+ ссылки на локальные переменные
				_op.f[params_count] = TBytecodeOp::GET_ARR_ELEMENT_CLASS_ID;
				_op.id[params_count] = source.NameId();
				source.GetToken();
				params_count++;
				source.GetToken(TTokenType::RParenth);
			} else {
				params[params_count++] = source.GetAttrib();
				source.GetToken();
			}
			if (params_count > 4)
				source.Error("—лишком много параметров!");
			if (!source.TestAndGet(TTokenType::Comma))
				break;
		}
		int params_count_info = GetBytecodeParamsCount((TOpcode::Enum) type);
		if (params_count != params_count_info) {
			char buf[256];
			sprintf(
					buf,
					"Ќеправильное количество параметров: дл€ %s надо %i параметра!",
					GetBytecodeString((TOpcode::Enum) type), params_count_info);
			source.Error(buf);
		}
		TOp op;
		int t = 0;
		op.type = (TOpcode::Enum) type;
		if (GetBytecodeParamExists((TOpcode::Enum) type, 0))
			op.f1 = params[t++];
		if (GetBytecodeParamExists((TOpcode::Enum) type, 1))
			op.f2 = params[t++];
		if (GetBytecodeParamExists((TOpcode::Enum) type, 2))
			op.v1 = params[t++];
		if (GetBytecodeParamExists((TOpcode::Enum) type, 3))
			op.v2 = params[t++];
		_op.op = op;
		code.Push(_op);
		source.GetToken(TTokenType::Semicolon);
	}
	source.GetToken(TTokenType::RBrace);
}

void TReturn::AnalyzeSyntax(TLexer& source) {
	InitPos(source);
	source.GetToken(TTokenType::ResWord, TResWord::Return);
	if (!source.Test(TTokenType::Semicolon))
		result.AnalyzeSyntax(source);
}

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

void TFor::AnalyzeSyntax(TLexer& source) {
	InitPos(source);

	compare.AnalyzeSyntax(source);
	source.GetToken(TTokenType::Semicolon);
	increment.AnalyzeStatement(source, false);
	source.GetToken(TTokenType::RParenth);
	statements.AnalyzeSyntax(source);
}

void TWhile::AnalyzeSyntax(TLexer& source) {
	InitPos(source);
	source.GetToken(TTokenType::ResWord, TResWord::While);
	source.GetToken(TTokenType::LParenth);

	compare.AnalyzeSyntax(source);
	source.GetToken(TTokenType::RParenth);
	statements.AnalyzeSyntax(source);
}

void TClassField::AnalyzeSyntax(TLexer& source) {
	InitPos(source);
	type.AnalyzeSyntax(source);
	is_static = source.TestAndGet(TTokenType::ResWord, TResWord::Static);
	if (is_static)
		need_push_this = false;
	TClassField* curr_field = this;
	do {
		if (curr_field != this) {
			curr_field->type = type;
			curr_field->is_static = is_static;
			curr_field->need_push_this = need_push_this;
			*(TTokenPos*) curr_field = *(TTokenPos*) this;
		}
		curr_field->name = source.NameId();
		source.GetToken(TTokenType::Identifier);
		if (source.Test(TTokenType::Comma)) {
			curr_field = new TClassField(owner);
			owner->AddField(curr_field);
		}
	} while (source.TestAndGet(TTokenType::Comma));
	source.GetToken(TTokenType::Semicolon);
}

void TParameter::AnalyzeSyntax(TLexer& source) {
	InitPos(source);
	type.AnalyzeSyntax(source);
	is_ref = source.TestAndGet(TTokenType::Ampersand);

	name = source.NameId();
	source.GetToken(TTokenType::Identifier);
}

bool ClassName(TLexer& source) {
	if (!source.TestAndGet(TTokenType::Identifier))
		return false;
	if (source.TestAndGet(TTokenType::Operator, TOperator::Less)) {
		while (!source.Test(TTokenType::Operator, TOperator::Greater)) {
			if (!ClassName(source))
				return false;
			if (!source.Test(TTokenType::Operator, TOperator::Greater))
				source.GetToken(TTokenType::Comma);
		}
		if (!source.TestAndGet(TTokenType::Operator, TOperator::Greater))
			return false;
	}
	if (source.TestAndGet(TTokenType::Dot)) {
		return ClassName(source);
	}
	return true;
}

bool ArrayDimensions(TLexer& source) {
	if (source.Test(TTokenType::LBracket) || source.Test(TTokenType::Operator,
			TOperator::GetArrayElement)) {
		while (true) {
			if (source.TestAndGet(TTokenType::LBracket)) {
				if (!source.TestAndGet(TTokenType::Value, TValue::Int))
					return false;
				source.GetToken(TTokenType::RBracket);
			} else if (source.TestAndGet(TTokenType::Operator,
					TOperator::GetArrayElement)) {
			} else
				break;
		}
	}
	return true;
}

bool IsVarDecl(TLexer& source) {
	int t = source.GetCurrentToken();
	bool result = ClassName(source);
	result = result && ArrayDimensions(source);
	result = result && source.Test(TTokenType::Identifier);//после типа объ€вл€емой переменной следует им€(имена) переменных
	source.SetCurrentToken(t);
	return result;
}

TOpArray TStatement::BuildLocalsAndParamsDestructor(
		TNotOptimizedProgram &program, int &deallocate_size) {
	return parent->BuildLocalsAndParamsDestructor(program, deallocate_size);
}

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
		source.Error("ќжидалс€ return,if,for,bytecode или this!");
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
		ret_ref = source.TestAndGet(TTokenType::Ampersand);//TODO проверка ссылки после типа а то не пон€тно почему ошибка
		ret.AnalyzeSyntax(source);
	}
}

void TMethod::AnalyzeSyntax(TLexer& source, bool realization) {
	InitPos(source);
	source.TestToken(TTokenType::ResWord);
	member_type = (TClassMember::Enum) source.Token();
	if (!IsIn(member_type, TClassMember::Func, TClassMember::Conversion))
		source.Error(
				"ќжидалось объ€вление метода,конструктора,деструктора,оператора или приведени€ типа!");
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
				"ќжидалось объ€вление метода,конструктора,деструктора,оператора или приведени€ типа!");
	}
	//
	if (!realization) {
		if (source.NameId() != owner->GetName())
			Error("ќжидалось им€ базового класса!");
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
			source.Error("ƒл€ внешнего метода тело должно отсутствовать!");
		source.GetToken(TTokenType::Semicolon);
	} else if (realization) {
		one_instruction = !source.Test(TTokenType::LBrace);
		statements.AnalyzeSyntax(source);
	}
}

void TClass::AccessDecl(TLexer& source, bool& readonly,
		TTypeOfAccess::Enum& access) {
	if (source.Type() == TTokenType::ResWord) {
		switch (source.Token()) {
		case TResWord::Readonly:
			source.GetToken();
			source.GetToken(TTokenType::Colon);
			readonly = true;
			access = TTypeOfAccess::Public;
			break;
		case TResWord::Public:
			source.GetToken();
			source.GetToken(TTokenType::Colon);
			readonly = false;
			access = TTypeOfAccess::Public;
			break;
		case TResWord::Protected:
			source.GetToken();
			source.GetToken(TTokenType::Colon);
			readonly = false;
			access = TTypeOfAccess::Protected;
			break;
		case TResWord::Private:
			source.GetToken();
			source.GetToken(TTokenType::Colon);
			readonly = false;
			access = TTypeOfAccess::Private;
			break;
		}
	}
}
