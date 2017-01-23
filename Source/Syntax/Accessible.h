#pragma once

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
private:
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
	void SetFactorId(Lexer::TNameId id);
	Lexer::TNameId GetFactorId();
	bool HasSizeMultiplierId();
	TMultifield();
	void SetSizeMultiplier(int mul);
	bool HasSizeMultiplier();
	int GetSizeMultiplier();
};