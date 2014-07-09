#pragma once

#include <assert.h>

namespace TVariableType
{
	enum Enum
	{
		ClassField,
		Parameter,
		Local,
		This
	};
}

class TVariable
{
protected:
	TVariableType::Enum var_type;
public:
	TVariable(TVariableType::Enum use_var_type) :var_type(use_var_type){}
	TVariableType::Enum GetType()const{
		return var_type;
	}
};

class TNodeWithOffset
{
	int offset;
	bool offset_initialized;
public:
	TNodeWithOffset()
	{
		offset = 0;
		offset_initialized = false;
	}
	void SetOffset(int use_offset)
	{
		assert(!offset_initialized);
		offset = use_offset;
		offset_initialized = true;
	}
	int GetOffset()
	{
		assert(offset_initialized);
		return offset;
	}
	bool IsOffsetInitialized()
	{
		return offset_initialized;
	}
};