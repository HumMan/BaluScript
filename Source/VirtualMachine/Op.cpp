#include "Op.h"

void TOp::Init()
{
	type = (TOpcode::Enum)TOpcode::NONE;
	f1 = -1;
	f2 = -1;
	v1 = -1;
	v2 = -1;
}

bool TOp::operator==(TOp v)
{
	return type == v.type&&v1 == v.v1&&v2 == v.v2;
}
TOp::TOp(){
	Init();
}
TOp::TOp(TOpcode::Enum op)
{
	Init();
	assert(ValidateOpParams(op, 0, 0, 0, 0));
	type = op;
}
TOp::TOp(TOpcode::Enum op, int use_v1)
{
	Init();
	assert(ValidateOpParams(op, 0, 0, 1, 0));
	type = op;
	v1 = use_v1;
}
TOp::TOp(TOpcode::Enum op, int use_v1, int use_v2)
{
	Init();
	assert(ValidateOpParams(op, 0, 0, 1, 1));
	type = op;
	v1 = use_v1;
	v2 = use_v2;
}
TOp::TOp(TOpcode::Enum op, char use_f1, int use_v1)
{
	Init();
	assert(ValidateOpParams(op, 1, 0, 1, 0));
	type = op;
	f1 = use_f1;
	v1 = use_v1;
}
TOp::TOp(TOpcode::Enum op, char use_f1, char use_f2, int use_v1)
{
	Init();
	assert(ValidateOpParams(op, 1, 1, 1, 0));
	type = op;
	f1 = use_f1;
	f2 = use_f2;
	v1 = use_v1;
}
TOp::TOp(TOpcode::Enum op, int use_f1, int use_f2, int use_v1, int use_v2)
{
	Init();
	assert(ValidateOpParams(op, 1, 1, 1, 1));
	type = op;
	f1 = use_f1;
	f2 = use_f2;
	v1 = use_v1;
	v2 = use_v2;
}