#pragma once

#include <assert.h>

enum class TVariableType
{
	ClassField,
	Parameter,
	Local,
	This,
	TemplateValue
};

class TVariable
{
protected:
	TVariableType var_type;
public:
	TVariable(TVariableType use_var_type) :var_type(use_var_type){}
	TVariableType GetType()const{
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