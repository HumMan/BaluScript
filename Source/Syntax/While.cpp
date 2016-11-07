#include "While.h"

#include "Expression.h"
#include "Statements.h"
#include "Class.h"
#include "ClassField.h"

#include "../Syntax/Method.h"

using namespace Lexer;

void TWhile::AnalyzeSyntax(Lexer::ILexer* source) {
	InitPos(source);
	source->GetToken(TTokenType::ResWord, TResWord::While);
	source->GetToken(TTokenType::LParenth);

	compare->AnalyzeSyntax(source);
	source->GetToken(TTokenType::RParenth);
	statements->AnalyzeSyntax(source);
}

TWhile::TWhile(TClass* use_owner, TMethod* use_method, TStatements* use_parent, int use_stmt_id)
	:TStatement(TStatementType::While, use_owner, use_method, use_parent, use_stmt_id)
	, compare(new TExpression(use_owner, use_method, use_parent, use_stmt_id))
	, statements(new TStatements(use_owner, use_method, use_parent, use_stmt_id))
{}

void TWhile::Accept(TStatementVisitor* visitor)
{
	visitor->Visit(this);
}