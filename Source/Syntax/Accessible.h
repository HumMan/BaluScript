#pragma once

#include <assert.h>

#include "../lexer.h"

namespace TTypeOfAccess
{
	enum Enum
	{
		Private,
		Protected,
		Public
	};
}

class TAccessible
{
protected:
	TTypeOfAccess::Enum access;
public:
	TAccessible();
	TTypeOfAccess::Enum GetAccess()const;
	void SetAccess(TTypeOfAccess::Enum use_access);
};

class TMultifield
{
	int size_multiplier;
	bool size_multiplier_set;
	TNameId factor_id;
public:
	void SetFactorId(TNameId id)
	{
		assert(factor_id.IsNull());
		factor_id = id;
	}
	TNameId GetFactorId()
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