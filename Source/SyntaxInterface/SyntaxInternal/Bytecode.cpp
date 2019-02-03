
#include "Bytecode.h"

#include "../../lexer.h"
#include <stdio.h>

#include "Method.h"
#include "Statements.h"

using namespace Lexer;
using namespace SyntaxInternal;

TBytecode::TBytecode(TClass* use_owner, TMethod* use_method, TStatements* use_parent, int use_stmt_id)
	:TStatement(SyntaxApi::TStatementType::Bytecode, use_owner, use_method, use_parent, use_stmt_id)
{
	//TODO
	//method->SetHasReturn(true);
}

void TBytecode::AnalyzeSyntax(Lexer::ILexer* source) {
	InitPos(source);
	source->GetToken(TResWord::Bytecode);
	source->GetToken(TTokenType::LBrace);
	while (source->Test(TTokenType::Bytecode)) {
		int type = source->Token();
		source->GetToken();
		code.push_back((TOpcode::Enum) type);
		source->GetToken(TTokenType::Semicolon);
	}
	source->GetToken(TTokenType::RBrace);
}

void TBytecode::Accept(SyntaxApi::IStatementVisitor* visitor)
{
	visitor->Visit(this);
}

const std::vector<TOpcode::Enum>& TBytecode::GetBytecode()const
{
	return code;
}