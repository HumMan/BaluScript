#pragma once

#include "SType.h"

class TClassField;

class TSClassField :public TSyntaxNode<TClassField>
{
	TSClass* owner;
	TSType type;
	bool linked;
public:
	TSClassField(TSClass* use_owner, TClassField* use_syntax);
	TSClass* GetClass()const;
	TSClass* GetOwner()const;
	void Link();
};