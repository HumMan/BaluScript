#include <stdlib.h>

#include "baseOps.h"

#include "../NativeTypes/DynArray.h"
#include "../NativeTypes/StaticArray.h"
#include "../NativeTypes/String.h"
#include "../NativeTypes/vec.h"

typedef BaluLib::TVec2 TVec2;
typedef BaluLib::TVec2i TVec2i;

#define BALU_SCRIPT_OPCODE_BEGIN( ENUM_NAME ) \
	const char* OpcodeInfo[]= 
#define BALU_SCRIPT_OPCODE_ELEM( element ) "ASM_"#element

#include "opcodes.h"

#undef BALU_SCRIPT_OPCODE_ELEM
#undef BALU_SCRIPT_OPCODE_BEGIN

const char* GetBytecodeString(TOpcode::Enum use_bytecode)
{
	return OpcodeInfo[use_bytecode];
}

bool TSimpleOps::ExecuteVec2Ops(TOpcode::Enum op, int*& sp, int* object)
{
	using namespace TOpcode;
	//////////////////////////////////////////////////
	//int
	switch (op)
	{
	case VEC2_CONSTR:
		*(TVec2*)object = *(TVec2*)(sp - 1);
		sp -= 2; break;
		//////////////////////////////////////////////////
		//vec2
	case VEC2_PLUS_A:
		**(TVec2**)(sp - 2) += *(TVec2*)(sp - 1);
		sp -= 3; break;
	case VEC2_MINUS_A:
		**(TVec2**)(sp - 2) -= *(TVec2*)(sp - 1);
		sp -= 3; break;
	case VEC2_MUL_A:
		**(TVec2**)(sp - 2) *= *(TVec2*)(sp - 1);
		sp -= 3; break;
	case VEC2_DIV_A:
		**(TVec2**)(sp - 2) /= *(TVec2*)(sp - 1);
		sp -= 3; break;

	case VEC2_PLUS:
		*(TVec2*)(sp - 3) += *(TVec2*)(sp - 1);
		sp -= 2; break;
	case VEC2_MINUS:
		*(TVec2*)(sp - 3) -= *(TVec2*)(sp - 1);
		sp -= 2; break;
	case VEC2_MULT:
		*(TVec2*)(sp - 3) *= *(TVec2*)(sp - 1);
		sp -= 2; break;
	case VEC2_DIV:
		*(TVec2*)(sp - 3) /= *(TVec2*)(sp - 1);
		sp -= 2; break;

	case VEC2_UNARY_MINUS:
		((TVec2*)(sp - 1))->Inverse();
		break;
	case R_VEC2_UNARY_MINUS:
		sp++;
		*(TVec2*)(sp - 1) = -*((TVec2*)sp[-1]);
		break;

	case RV_VEC2_GET_ELEMENT:
		if (*sp != 0 && *sp != 1)throw "Ошибка доступа к элементу вектора!";
		*(float*)(sp - 1) = *((float*)(sp[-1]) + *sp);
		sp--; break;

	case VV_VEC2_GET_ELEMENT:
		if (*sp != 0 && *sp != 1)throw "Ошибка доступа к элементу вектора!";
		*(float*)(sp - 2) = ((float*)sp)[*sp - 2];
		sp -= 2; break;

	case VEC2_DISTANCE:
		*(float*)(sp - 3) = ((TVec2*)(sp - 3))->Distance(*(TVec2*)(sp - 1));
		sp -= 3; break;
	case VEC2_DOT:
		*(float*)(sp - 3) = (*((TVec2*)(sp - 3)))*(*(TVec2*)(sp - 1));
		sp -= 3; break;
	case VEC2_LENGTH:
		*(float*)(sp - 1) = ((TVec2*)(sp - 1))->Length();
		sp--; break;
	case VEC2_NORMALIZE:
		((TVec2*)(sp - 1))->Normalize();
		break;
	case VEC2_REFLECT:
		((TVec2*)(sp - 3))->Reflect(*(TVec2*)(sp - 1));
		sp -= 2; break;
	default:
		return false;
	}
	return true;
}
bool TSimpleOps::ExecuteVec2iOps(TOpcode::Enum op, int*& sp, int* object)
{
	using namespace TOpcode;
	//////////////////////////////////////////////////
	//int
	switch (op)
	{
	case VEC2I_CONSTR:
		*(TVec2i*)object = *(TVec2i*)(sp - 1);
		sp -= 2; break;
		//////////////////////////////////////////////////
		//vec2
	case VEC2I_PLUS_A:
		**(TVec2i**)(sp - 2) += *(TVec2i*)(sp - 1);
		sp -= 3; break;
	case VEC2I_MINUS_A:
		**(TVec2i**)(sp - 2) -= *(TVec2i*)(sp - 1);
		sp -= 3; break;
	case VEC2I_MUL_A:
		**(TVec2i**)(sp - 2) *= *(TVec2i*)(sp - 1);
		sp -= 3; break;
	case VEC2I_DIV_A:
		**(TVec2i**)(sp - 2) /= *(TVec2i*)(sp - 1);
		sp -= 3; break;

	case VEC2I_PLUS:
		*(TVec2i*)(sp - 3) += *(TVec2i*)(sp - 1);
		sp -= 2; break;
	case VEC2I_MINUS:
		*(TVec2i*)(sp - 3) -= *(TVec2i*)(sp - 1);
		sp -= 2; break;
	case VEC2I_MULT:
		*(TVec2i*)(sp - 3) *= *(TVec2i*)(sp - 1);
		sp -= 2; break;
	case VEC2I_DIV:
		*(TVec2i*)(sp - 3) /= *(TVec2i*)(sp - 1);
		sp -= 2; break;

	case VEC2I_UNARY_MINUS:
		((TVec2i*)(sp - 1))->Inverse();
		break;
	case R_VEC2I_UNARY_MINUS:
		sp++;
		*(TVec2i*)(sp - 1) = -*((TVec2i*)sp[-1]);
		break;

	case RV_VEC2I_GET_ELEMENT:
		if (*sp != 0 && *sp != 1)throw "Ошибка доступа к элементу вектора!";
		*(sp - 1) = *((int*)(sp[-1]) + *sp);
		sp--; break;

	case VV_VEC2I_GET_ELEMENT:
		if (*sp != 0 && *sp != 1)throw "Ошибка доступа к элементу вектора!";
		*(sp - 2) = sp[*sp - 2];
		sp -= 2; break;

	case VEC2I_DISTANCE:
		*(float*)(sp - 3) = ((TVec2i*)(sp - 3))->Distance(*(TVec2i*)(sp - 1));
		sp -= 3; break;
	case VEC2I_DOT:
		*(float*)(sp - 3) = ((TVec2i*)(sp - 3))->AbsScalarMul(*(TVec2i*)(sp - 1));
		sp -= 3; break;
	case VEC2I_LENGTH:
		*(float*)(sp - 1) = ((TVec2i*)(sp - 1))->Length();
		sp--; break;
	case VEC2I_NORMALIZE:
		((TVec2i*)(sp - 1))->Normalize();
		break;
	case VEC2I_REFLECT:
		((TVec2i*)(sp - 3))->Reflect(*(TVec2i*)(sp - 1));
		sp -= 2; break;
	default:
		return false;
	}
	return true;
}

void Compare(int* &sp)
{
	int *s, *d;
	s = &sp[0];
	d = &s[-1];
	sp--;
	*sp = ((*s) == (*d));
}

bool TSimpleOps::ExecuteBoolOps(TOpcode::Enum op, int*& sp, int* object)
{
	using namespace TOpcode;
	//////////////////////////////////////////////////
	//int
	switch (op)
	{
	case BOOL_CONSTR:
		*object = *(bool*)&sp[0];
		sp--; break;
		//////////////////////////////////////////////////
		//bool
	case BOOL_AND:
		sp[-1] = *(bool*)&sp[-1] && *(bool*)&sp[0];
		sp--; break;
	case BOOL_OR:
		sp[-1] = *(bool*)&sp[-1] || *(bool*)&sp[0];
		sp--; break;
	case BOOL_NOT:
		sp[0] = !*(bool*)&sp[0]; break;
	case EQUAL:
		Compare(sp);
		break;
	case NOT_EQUAL:
		Compare(sp);
		sp[0] = (!*(bool*)&sp[0]);
		break;
	default:
		return false;
	}
	return true;
}
bool TSimpleOps::ExecuteFloatOps(TOpcode::Enum op, int*& sp, int* object)
{
	using namespace TOpcode;
	//////////////////////////////////////////////////
	//int
	switch (op)
	{
		//////////////////////////////////////////////////
	case FLOAT_CONSTR:
		*object = sp[0];
		sp--; break;
		//////////////////////////////////////////////////
		//float
	case FLOAT_PRINT:
		printf("print: %f\n", *(float*)sp);
		sp--; break;
	case FLOAT_PLUS:
		*(float*)(sp - 1) += *(float*)sp;
		sp--; break;
	case FLOAT_MINUS:
		*(float*)(sp - 1) -= *(float*)sp;
		sp--; break;
	case FLOAT_DIV:
		*(float*)(sp - 1) /= *(float*)sp;
		sp--; break;
	case FLOAT_MULT:
		*(float*)(sp - 1) *= *(float*)sp;
		sp--; break;

	case FLOAT_UNARY_MINUS:
		*((float*)sp) = -*((float*)sp); break;
	case R_FLOAT_UNARY_MINUS:
		*((float*)sp) = -*((float*)*sp); break;

	case FLOAT_PLUS_A:
		*((float*)sp[-1]) += *((float*)sp);
		sp -= 2; break;
	case FLOAT_MINUS_A:
		*((float*)sp[-1]) -= *((float*)sp);
		sp -= 2; break;
	case FLOAT_MUL_A:
		*((float*)sp[-1]) *= *((float*)sp);
		sp -= 2; break;
	case FLOAT_DIV_A:
		*((float*)sp[-1]) /= *((float*)sp);
		sp -= 2; break;


	case FLOAT_LESS:
		sp[-1] = (*(float*)(sp - 1) < *(float*)sp);
		sp--; break;
	case FLOAT_LESS_EQ:
		sp[-1] = (*(float*)(sp - 1) <= *(float*)sp);
		sp--; break;
	case FLOAT_GREATER:
		sp[-1] = (*(float*)(sp - 1) > *(float*)sp);
		sp--; break;
	case FLOAT_GREATER_EQ:
		sp[-1] = (*(float*)(sp - 1) >= *(float*)sp);
		sp--; break;


	case FLOAT_TO_BOOL:
		*sp = (*(float*)sp != 0.0f); break;
	case R_FLOAT_TO_BOOL:
		*sp = (*(float*)*sp != 0.0f); break;
	case FLOAT_TO_INT:
		*sp = int(*(float*)sp); break;
	case FLOAT_TO_VEC2:
		sp++;
		*(float*)sp = *(float*)(sp - 1); break;

	case FLOAT_TO_VEC2I:
		*(int*)sp = *(float*)(sp);
		sp++;
		*(int*)sp = *(sp - 1); break;

	case FLOAT_ABS:
		*(float*)sp = fabs(*(float*)sp); break;
	case FLOAT_ASIN:
		*(float*)sp = asin(*(float*)sp); break;
	case FLOAT_ACOS:
		*(float*)sp = acos(*(float*)sp); break;
	case FLOAT_ATAN:
		*(float*)sp = atan(*(float*)sp); break;
	case FLOAT_CEIL:
		*(float*)sp = ceil(*(float*)sp); break;
	case FLOAT_CLAMP:
		*(float*)(sp - 2) = BaluLib::Clamp(*(float*)(sp - 2), *(float*)(sp - 1), *(float*)(sp));
		sp -= 2; break;
	case FLOAT_COS:
		*(float*)sp = cos(*(float*)sp); break;
	case FLOAT_DEG_TO_RAD:
		*(float*)sp = (*(float*)sp)*(float(M_PI) / 180.0f); break;
	case FLOAT_EXP:
		*(float*)sp = exp(*(float*)sp); break;
	case FLOAT_FLOOR:
		*(float*)sp = floor(*(float*)sp); break;
	case FLOAT_FRAC:
		*(float*)sp -= floor(*(float*)sp); break;
	case FLOAT_LOG:
		*(float*)sp = log(*(float*)sp); break;
	case FLOAT_LOG2:
		*(float*)sp = log(*(float*)sp) / log(2.0f); break;
	case FLOAT_LOG10:
		*(float*)sp = log10(*(float*)sp); break;
	case FLOAT_MAX:
		((float*)sp)[-1] = ((float*)sp)[-1] < ((float*)sp)[0] ? ((float*)sp)[0] : ((float*)sp)[-1];
		sp--; break;
	case FLOAT_MIN:
		((float*)sp)[-1] = ((float*)sp)[-1] > ((float*)sp)[0] ? ((float*)sp)[0] : ((float*)sp)[-1];
		sp--; break;
	case FLOAT_PI:
		((float*)sp)[0] = float(M_PI);
		sp--; break;
	case FLOAT_POW:
		((float*)sp)[-1] = pow(((float*)sp)[-1], ((float*)sp)[0]);
		sp--; break;
	case FLOAT_RAD_TO_DEG:
		*(float*)sp = (*(float*)sp)*(180.0f / float(M_PI)); break;
	case FLOAT_RAND:
		*(float*)(++sp) = BaluLib::Randf(); break;
	case FLOAT_SIGN:
		*(float*)sp = (*(float*)sp) > 0.0f ? 1.0f : ((*(float*)sp) == 0.0f ? 0.0f : -1.0f); break;
	case FLOAT_SIN:
		*(float*)sp = sin(*(float*)sp); break;
	case FLOAT_SQRT:
		*(float*)sp = sqrt(*(float*)sp); break;
	case FLOAT_SQR:
		*(float*)sp *= *(float*)sp; break;
	case FLOAT_TAN:
		*(float*)sp = tan(*(float*)sp); break;
	case FLOAT_TRUNC:
		*(float*)sp = float(int(*(float*)sp)); break;
		//////////////////////////////////////////////////
	default:
		return false;
	}
	return true;
}

bool TSimpleOps::ExecuteIntOps(TOpcode::Enum op, int*& sp, int* object)
{
	using namespace TOpcode;
	//////////////////////////////////////////////////
	//int
	switch (op)
	{
	case INT_CONSTR:
		*object = sp[0];
		sp--; break;
	case INT_PLUS:
		sp[-1] += sp[0];
		sp--; break;
	case INT_MINUS:
		sp[-1] -= sp[0];
		sp--; break;
	case INT_DIV:
		sp[-1] /= sp[0];
		sp--; break;
	case INT_MUL:
		sp[-1] *= sp[0];
		sp--; break;
	case INT_MOD:
		sp[-1] %= sp[0];
		sp--; break;

	case INT_UNARY_MINUS:
		*sp = -*sp; break;
	case R_INT_UNARY_MINUS:
		*sp = -*((int*)*sp); break;
	case INT_PREFIX_DEC:
		*(int*)sp[0] = *(int*)sp[0] - 1; break;
		sp--;
	case INT_PREFIX_INC:
		*(int*)sp[0] = *(int*)sp[0] + 1; break;
		sp--;
	case INT_TO_VEC2:
		sp++;
		*((TVec2*)(sp - 1)) = TVec2(float(sp[-1]));
		break;
	case INT_TO_VEC2I:
		sp++;
		*((TVec2i*)(sp - 1)) = TVec2i(sp[-1]);
		break;
	case INT_PLUS_A:
		*((int*)sp[-1]) += *sp;
		sp -= 2; break;
	case INT_MINUS_A:
		*((int*)sp[-1]) -= *sp;
		sp -= 2; break;
	case INT_MUL_A:
		*((int*)sp[-1]) *= *sp;
		sp -= 2; break;
	case INT_DIV_A:
		*((int*)sp[-1]) /= *sp;
		sp -= 2; break;
	case INT_MOD_A:
		*((int*)sp[-1]) %= *sp;
		sp -= 2; break;

	case INT_LESS:
		sp[-1] = (sp[-1] < *sp);
		sp--; break;
	case INT_LESS_EQ:
		sp[-1] = (sp[-1] <= *sp);
		sp--; break;
	case INT_GREATER:
		sp[-1] = (sp[-1] > *sp);
		sp--; break;
	case INT_GREATER_EQ:
		sp[-1] = (sp[-1] >= *sp);
		sp--; break;

	case INT_TO_FLOAT:
		*(float*)sp = float(*sp); break;
	case R_INT_TO_FLOAT:
		*(float*)sp = float(*(int*)(*sp)); break;
	case INT_TO_BOOL:
		*sp = (*sp != 0); break;
	case R_INT_TO_BOOL:
		*sp = (*(int*)*sp != 0); break;

	case INT_ABS:
		*sp = abs(*sp); break;
	case INT_MAXIMUM:
		sp[-1] = (sp[-1] < sp[0]) ? sp[0] : sp[-1];
		sp--; break;
	case INT_MINIMUM:
		sp[-1] = (sp[-1] > sp[0]) ? sp[0] : sp[-1];
		sp--; break;
	case INT_SIGN:
		*sp = *sp > 0 ? 1 : (*sp == 0 ? 0 : -1); break;
	default:
		return false;
	}
	return true;
}
