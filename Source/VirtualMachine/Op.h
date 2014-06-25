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
	void Init()
	{
		type = (TOpcode::Enum)TOpcode::NONE;
		f1=-1;
		f2=-1;
		v1=-1;
		v2=-1;
	}
public:
	TOpcode::Enum type;
	char f1,f2;
	int v1;
	int v2;
	bool operator==(TOp v)
	{
		return type==v.type&&v1==v.v1&&v2==v.v2;
	}
	TOp(){
		Init();
	}
	TOp(TOpcode::Enum op)
	{
		Init();
		assert(ValidateOpParams(op,0,0,0,0));
		type=op;
	}
	TOp(TOpcode::Enum op,int use_v1)
	{
		Init();
		assert(ValidateOpParams(op,0,0,1,0));
		type=op;
		v1=use_v1;
	}
	TOp(TOpcode::Enum op,int use_v1,int use_v2)
	{
		Init();
		assert(ValidateOpParams(op,0,0,1,1));
		type=op;
		v1=use_v1;
		v2=use_v2;
	}
	TOp(TOpcode::Enum op,char use_f1,int use_v1)
	{
		Init();
		assert(ValidateOpParams(op,1,0,1,0));
		type=op;
		f1=use_f1;
		v1=use_v1;
	}
	TOp(TOpcode::Enum op,char use_f1,char use_f2,int use_v1)
	{
		Init();
		assert(ValidateOpParams(op,1,1,1,0));
		type=op;
		f1=use_f1;
		f2=use_f2;
		v1=use_v1;
	}
	TOp(TOpcode::Enum op,int use_f1,int use_f2,int use_v1,int use_v2)
	{
		Init();
		assert(ValidateOpParams(op,1,1,1,1));
		type=op;
		f1=use_f1;
		f2=use_f2;
		v1=use_v1;
		v2=use_v2;
	}
};