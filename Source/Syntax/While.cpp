#include "While.h"

#include "Class.h"
#include "ClassField.h"

#include "../Syntax/Method.h"

using namespace Lexer;
using namespace SyntaxInternal;

void TWhile::AnalyzeSyntax(Lexer::ILexer* source) {
	InitPos(source);
	source->GetToken(TResWord::While);
	source->GetToken(TTokenType::LParenth);

	compare->AnalyzeSyntax(source);
	source->GetToken(TTokenType::RParenth);
	statements->AnalyzeSyntax(source);
}

TWhile::TWhile(TClass* use_owner, TMethod* use_method, TStatements* use_parent, int use_stmt_id)
	:TStatement(SyntaxApi::TStatementType::While, use_owner, use_method, use_parent, use_stmt_id)
	, compare(new TExpression(use_owner, use_method, use_parent, use_stmt_id))
	, statements(new TStatements(use_owner, use_method, use_parent, use_stmt_id))
{}

void TWhile::Accept(SyntaxApi::IStatementVisitor* visitor)
{
	visitor->Visit(this);
}

TExpression* TWhile::GetCompare()const
{
	return compare.get();
}
TStatements* TWhile::GetStatements()const
{
	return statements.get();
}