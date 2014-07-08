#include "For.h"

#include "Expression.h"
#include "Statements.h"
#include "ClassField.h"

void TFor::AnalyzeSyntax(TLexer& source) {
	InitPos(source);

	compare->AnalyzeSyntax(source);
	source.GetToken(TTokenType::Semicolon);
	increment->AnalyzeStatement(source, false);
	source.GetToken(TTokenType::RParenth);
	statements->AnalyzeSyntax(source);
}
TFor::TFor(TClass* use_owner, TMethod* use_method, TStatements* use_parent, int use_stmt_id)
	:TStatement(TStatementType::For, use_owner, use_method, use_parent, use_stmt_id)
	, compare(new TExpression(use_owner, use_method, use_parent, use_stmt_id))
	, increment(new TStatements(use_owner, use_method, use_parent, use_stmt_id))
	, statements(new TStatements(use_owner, use_method, use_parent, use_stmt_id))
{
}

void TFor::Accept(TStatementVisitor* visitor)
{
	visitor->Visit(this);
}