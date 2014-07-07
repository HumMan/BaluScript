#include "SClassField.h"

#include "SType.h"

#include "../Syntax/ClassField.h"

TSClassField::TSClassField(TSClass* use_owner, TClassField* use_syntax) 
	:TSyntaxNode(use_syntax), TVariable(TVariableType::ClassField)
	, type(use_owner, &use_syntax->type)
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

void TSClassField::Link()
{

	type.Link();
}
