#include "SClassField.h"

TSClass* TSClassField::GetClass()const
{
	return type.GetClass();
}

TSClass* TSClassField::GetOwner()const
{
	return owner;
}
