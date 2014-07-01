#pragma once

#include "SType.h"

class TClassField;

class TSClassField :public TSyntaxNode<TClassField>
{
	TSClass* owner;
	TSType type;
public:
	TSClass* GetClass()const;
	TSClass* GetOwner()const;
};