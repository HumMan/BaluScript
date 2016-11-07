#include "If.h"

#include "Statements.h"
#include "Expression.h"
#include "ClassField.h"

using namespace Lexer;

void TIf::AnalyzeSyntax(Lexer::ILexer* source) {
	InitPos(source);
	source->GetToken(TTokenType::ResWord, TResWord::If);
	source->GetToken(TTokenType::LParenth);

	bool_expr->AnalyzeSyntax(source);
	source->GetToken(TTokenType::RParenth);
	statements->AnalyzeSyntax(source);
	if (source->Test(TTokenType::ResWord, TResWord::Else)) {
		source->GetToken(TTokenType::ResWord, TResWord::Else);
		else_statements->AnalyzeSyntax(source);
	}
}

TIf::TIf(TClass* use_owner, TMethod* use_method, TStatements* use_parent, int use_stmt_id)
	:TStatement(TStatementType::If, use_owner, use_method, use_parent, use_stmt_id)
	, bool_expr(new TExpression(use_owner, use_method, use_parent, use_stmt_id))
	, statements(new TStatements(use_owner, use_method, use_parent, use_stmt_id))
	, else_statements(new TStatements(use_owner, use_method, use_parent, use_stmt_id))
{}

void TIf::Accept(TStatementVisitor* visitor)
{
	visitor->Visit(this);
}