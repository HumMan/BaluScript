#pragma once

#include <assert.h>

#define BALU_SCRIPT_OPCODE_BEGIN( ENUM_NAME ) enum ENUM_NAME//:unsigned char
#define BALU_SCRIPT_OPCODE_ELEM4( element ,p0,p1,p2,p3) element
#define BALU_SCRIPT_OPCODE_ELEM2( element ,p2,p3) element
#define BALU_SCRIPT_OPCODE_ELEM0( element ) element
#define BALU_SCRIPT_OPCODE_END() ;

namespace TOpcode{
#include "opcodes.h"
}

#undef BALU_SCRIPT_OPCODE_ELEM4
#undef BALU_SCRIPT_OPCODE_ELEM2
#undef BALU_SCRIPT_OPCODE_ELEM0
#undef BALU_SCRIPT_OPCODE_BEGIN
#undef BALU_SCRIPT_OPCODE_END

extern bool ValidateOpParams(TOpcode::Enum op, bool f1, bool f2, bool v1, bool v2);
extern char* GetBytecodeString(TOpcode::Enum use_bytecode);
extern int GetBytecodeParamsCount(TOpcode::Enum use_bytecode);
extern bool GetBytecodeParamExists(TOpcode::Enum use_bytecode, int i);

struct TOp
{
private:
	void Init();
public:
	TOpcode::Enum type;
	char f1,f2;
	int v1;
	int v2;
	bool operator==(TOp v);
	TOp();
	TOp(TOpcode::Enum op);
	TOp(TOpcode::Enum op, int use_v1);
	TOp(TOpcode::Enum op, int use_v1, int use_v2);
	TOp(TOpcode::Enum op, char use_f1, int use_v1);
	TOp(TOpcode::Enum op, char use_f1, char use_f2, int use_v1);
	TOp(TOpcode::Enum op, int use_f1, int use_f2, int use_v1, int use_v2);
};