#include "SBytecode.h"

#include "SMethod.h"
#include "SClass.h"

#include "../../BytecodeBaseOps/baseOps.h"

TSBytecode::TSBytecode(TSClass* use_owner, TSMethod* use_method, TSStatements* use_parent, SyntaxApi::IBytecode* use_syntax)
	:TSStatement(SyntaxApi::TStatementType::Bytecode, use_owner, use_method, use_parent, dynamic_cast<SyntaxApi::IStatement*>(use_syntax))
{

}

SyntaxApi::IBytecode* TSBytecode::GetSyntax()const
{
	return dynamic_cast<SyntaxApi::IBytecode*>(TSyntaxNode::GetSyntax());
}

void TSBytecode::Build(SemanticApi::TGlobalBuildContext build_context)
{
}

const std::vector<TOpcode::Enum>& TSBytecode::GetBytecode() const
{
	return GetSyntax()->GetBytecode();
}

void TSBytecode::Accept(SemanticApi::ISStatementVisitor * visitor)
{
	visitor->Visit(this);
}

