#pragma once

#include <assert.h>
#include <memory.h>
#include <string.h>
#include "Op.h"

class TProgram;

class TSimpleOps
{
public:
	static bool ExecuteIntOps(TOpcode::Enum op, int*& sp, int* object);
	static bool ExecuteFloatOps(TOpcode::Enum op, int*& sp, int* object);
	static bool ExecuteBoolOps(TOpcode::Enum op, int*& sp, int* object);
	static bool ExecuteVec2Ops(TOpcode::Enum op, int*& sp, int* object);
	static bool ExecuteVec2iOps(TOpcode::Enum op, int*& sp, int* object);
};