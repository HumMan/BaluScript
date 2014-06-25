#include "Statements.h"

#include "Return.h"
#include "If.h"
#include "For.h"
#include "Bytecode.h"
#include "LocalVar.h"

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
			}
			else if (source.TestAndGet(TTokenType::Operator,
				TOperator::GetArrayElement)) {
			}
			else
				break;
		}
	}
	return true;
}

bool IsVarDecl(TLexer& source) {
	int t = source.GetCurrentToken();
	bool result = ClassName(source);
	result = result && ArrayDimensions(source);
	result = result && source.Test(TTokenType::Identifier);//����� ���� ����������� ���������� ������� ���(�����) ����������
	source.SetCurrentToken(t);
	return result;
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
				source.Error("return ����� ������������ ������ ��� ��������� ����������!");
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
		source.Error("������� return,if,for,bytecode ��� this!");
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
	assert(false);//�� ���� switch ������ ���� return 
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

void TStatements::AddVar(TLocalVar* use_var) {
	statement.Push((TStatement*)use_var);
	use_var->SetStmtId(statement.GetHigh());
	var_declarations.Push(TVarDecl(statement.GetHigh(), use_var));
}

void TStatements::Add(TStatement* use_statement){
	statement.Push(use_statement);
	use_statement->SetStmtId(statement.GetHigh());//TODO �� �����
}

TStatement* TStatements::GetStatement(int i)
{
	return statement[i];
}
TStatement* TStatements::GetCopy()
{
	TStatements* copy = new TStatements(*this);
	return copy;
}

void TStatements::InitOwner(TClass* use_owner, TMethod* use_method, TStatements* use_parent)
{
	TStatement::_InitOwner(use_owner, use_method, use_parent);
	for (int i = 0; i <= statement.GetHigh(); i++)
		statement[i]->InitOwner(use_owner, use_method, this);
}
int TStatements::GetHigh()
{
	return statement.GetHigh();
}
TStatements::TStatements(TClass* use_owner, TMethod* use_method, TStatements* use_parent, int use_stmt_id)
	:TStatement(TStatementType::Statements, use_owner, use_method, use_parent, use_stmt_id)
{
}
TStatements::~TStatements()
{
	for (int i = 0; i <= statement.GetHigh(); i++)delete statement[i];
}