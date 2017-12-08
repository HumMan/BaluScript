#include "SClassField.h"

#include "SType.h"

TSClassField::TSClassField(TSClass* use_owner, SyntaxApi::IClassField* use_syntax) 
	:TSyntaxNode(use_syntax), TVariable(TVariableType::ClassField)
	, type(use_owner, use_syntax->GetType())
{
	owner = use_owner;
}

TSClass* TSClassField::GetClass()const
{
	return type.GetClass();
}

TSClass* TSClassField::GetOwner()const
{
	return owner;
}

void TSClassField::LinkSignature(TGlobalBuildContext build_context)
{

	type.LinkSignature(build_context);
}

void TSClassField::LinkBody(TGlobalBuildContext build_context)
{

	type.LinkBody(build_context);
}

