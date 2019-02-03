#pragma once

#include <assert.h>

#define BALU_SCRIPT_OPCODE_BEGIN( ENUM_NAME ) enum ENUM_NAME
#define BALU_SCRIPT_OPCODE_ELEM( element ) element

namespace TOpcode{
#include "opcodes.h"
}

#undef BALU_SCRIPT_OPCODE_ELEM
#undef BALU_SCRIPT_OPCODE_BEGIN

extern const char* GetBytecodeString(TOpcode::Enum use_bytecode);