﻿#pragma once

#include <assert.h>

#include "../lexer.h"

enum class TTypeOfAccess
{
	Private,
	Protected,
	Public
};

class TAccessible
{
protected:
	TTypeOfAccess access;
public:
	TAccessible();
	TTypeOfAccess GetAccess()const;
	void SetAccess(TTypeOfAccess use_access);
};

class TMultifield
{
	int size_multiplier;
	bool size_multiplier_set;
	Lexer::TNameId factor_id;
public:
	void SetFactorId(Lexer::TNameId id)
	{
		assert(factor_id.IsNull());
		factor_id = id;
	}
	Lexer::TNameId GetFactorId()
	{
		assert(!factor_id.IsNull());
		return factor_id;
	}
	bool HasSizeMultiplierId()
	{
		return !factor_id.IsNull();
	}
	TMultifield()
	{
		size_multiplier = -1;
		size_multiplier_set = false;
	}
	void SetSizeMultiplier(int mul)
	{
		assert(!size_multiplier_set);
		size_multiplier = mul;
		size_multiplier_set = true;
	}
	bool HasSizeMultiplier()
	{
		return size_multiplier_set;
	}
	int GetSizeMultiplier()
	{
		return size_multiplier;
	}
};