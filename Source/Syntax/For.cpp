#include "For.h"

#include "Expression.h"
#include "Statements.h"
#include "ClassField.h"

using namespace Lexer;
using namespace SyntaxInternal;

void TFor::AnalyzeSyntax(Lexer::ILexer* source) {
	InitPos(source);

	compare->AnalyzeSyntax(source);
	source->GetToken(TTokenType::Semicolon);
	increment->AnalyzeStatement(source, false);
	source->GetToken(TTokenType::RParenth);
	statements->AnalyzeSyntax(source);
}
TFor::TFor(TClass* use_owner, TMethod* use_method, TStatements* use_parent, int use_stmt_id)
	:TStatement(SyntaxApi::TStatementType::For, use_owner, use_method, use_parent, use_stmt_id)
	, compare(new TExpression(use_owner, use_method, use_parent, use_stmt_id))
	, increment(new TStatements(use_owner, use_method, use_parent, use_stmt_id))
	, statements(new TStatements(use_owner, use_method, use_parent, use_stmt_id))
{
}

void TFor::Accept(SyntaxApi::IStatementVisitor* visitor)
{
	visitor->Visit(this);
}

TExpression* TFor::GetCompare()const
{
	return compare.get();
}
TStatements* TFor::GetIncrement()const
{
	return increment.get();
}
TStatements* TFor::GetStatements()const
{
	return statements.get();
}