#pragma once

#include <assert.h>

#include "../SemanticTreeApi.h"

class TVariable: public SemanticApi::IVariable
{
private:
	SemanticApi::VariableType var_type;
public:
	TVariable(SemanticApi::VariableType use_var_type) :var_type(use_var_type){}
	SemanticApi::VariableType GetType()const{
		return var_type;
	}
};

class TNodeWithOffset: public virtual SemanticApi::INodeWithOffset
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
	int GetOffset()const
	{
		assert(offset_initialized);
		return offset;
	}
	bool IsOffsetInitialized()const
	{
		return offset_initialized;
	}
};