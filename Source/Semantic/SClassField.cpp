#include "SClassField.h"

#include "SType.h"

#include "../Syntax/ClassField.h"

TSClassField::TSClassField(TSClass* use_owner, TClassField* use_syntax) 
	:TSyntaxNode(use_syntax), TVariable(TVariableType::ClassField)
	, type(use_owner, use_syntax->type.get())
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

void TSClassField::LinkSignature(std::vector<TSClassField*>* static_fields, std::vector<TSLocalVar*>* static_variables)
{

	type.LinkSignature(static_fields, static_variables);
}

void TSClassField::LinkBody(std::vector<TSClassField*>* static_fields, std::vector<TSLocalVar*>* static_variables)
{

	type.LinkBody(static_fields, static_variables);
}

