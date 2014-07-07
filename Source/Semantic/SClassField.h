﻿#pragma once

#include "SType.h"
#include "Variable.h"

class TClassField;

class TSClassField :public TSyntaxNode<TClassField>, public TVariable
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