#include "SBytecode.h"

#include "SMethod.h"
#include "SClass.h"

#include "../../virtualMachine.h"

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
	auto code = GetSyntax()->GetBytecode();
	for (size_t i = 0; i<code.size(); i++)
	{
		if (code[i].f[0] == SyntaxApi::TBytecodeOp::GET_ARR_ELEMENT_CLASS_ID)
		{
			TSClass* temp = GetOwner()->GetClass(code[i].id[0]);
			if (temp == nullptr)GetSyntax()->Error("Неизвестный идентификатор!");
			//GetSyntax()->code[i].op.v1 = program.CreateArrayElementClassId(temp);
			array_element_classes.push_back(temp);
		}//TODO сделать нормально без повторений
		if (code[i].f[1] == SyntaxApi::TBytecodeOp::GET_ARR_ELEMENT_CLASS_ID)
		{
			TSClass* temp = GetOwner()->GetClass(code[i].id[1]);
			if (temp == nullptr)GetSyntax()->Error("Неизвестный идентификатор!");
			//code[i].op.v2 = program.CreateArrayElementClassId(temp);
			array_element_classes.push_back(temp);
		}//TODO сделать нормально без повторений
	}
}

const std::vector<SyntaxApi::TBytecodeOp>& TSBytecode::GetBytecode() const
{
	return GetSyntax()->GetBytecode();
}

void TSBytecode::Accept(SemanticApi::ISStatementVisitor * visitor)
{
	visitor->Visit(this);
}

