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
	result = result && source.Test(TTokenType::Identifier);//после типа объявляемой переменной следует имя(имена) переменных
	source.SetCurrentToken(t);
	return result;
}

