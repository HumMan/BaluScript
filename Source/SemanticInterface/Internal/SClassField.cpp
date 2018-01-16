#include "SClassField.h"

#include "SType.h"

#include "SClass.h"

TSClassField::TSClassField(TSClass* use_owner, SyntaxApi::IClassField* use_syntax) 
	:TSyntaxNode(use_syntax), TVariable(SemanticApi::VariableType::ClassField)
	, type(use_owner, use_syntax->GetType())
{
	owner = use_owner;
}

SemanticApi::ISClass* TSClassField::GetClass()const
{
	return type.GetClass();
}

SemanticApi::ISClass* TSClassField::GetOwner()const
{
	return owner;
}

void TSClassField::LinkSignature(SemanticApi::TGlobalBuildContext build_context)
{

	type.LinkSignature(build_context);
}

void TSClassField::LinkBody(SemanticApi::TGlobalBuildContext build_context)
{

	type.LinkBody(build_context);
}

bool TSClassField::IsStatic() const
{
	return GetSyntax()->IsStatic();
}

