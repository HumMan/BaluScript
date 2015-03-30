#include "virtualMachine.h"

#include "VirtualMachine/Program.h"

#include "NativeTypes/DynArray.h"
#include "NativeTypes/StaticArray.h"
#include "NativeTypes/String.h"

#define BALU_SCRIPT_OPCODE_BEGIN( ENUM_NAME ) \
struct TOpcodeInfo{char* name;int count;bool f1,f2,v1,v2;};\
	const TOpcodeInfo OpcodeInfo[]= 
#define BALU_SCRIPT_OPCODE_ELEM4( element ,p0,p1,p2,p3) {"ASM_"#element,p0+p1+p2+p3,p0,p1,p2,p3}
#define BALU_SCRIPT_OPCODE_ELEM2( element ,p2,p3) {"ASM_"#element,p2+p3,0,0,p2,p3}
#define BALU_SCRIPT_OPCODE_ELEM0( element ) {"ASM_"#element,0,0,0,0,0}
#define BALU_SCRIPT_OPCODE_END() ;

#include "VirtualMachine/opcodes.h"

#undef BALU_SCRIPT_OPCODE_ELEM4
#undef BALU_SCRIPT_OPCODE_ELEM2
#undef BALU_SCRIPT_OPCODE_ELEM0
#undef BALU_SCRIPT_OPCODE_BEGIN
#undef BALU_SCRIPT_OPCODE_END

char* GetBytecodeString(TOpcode::Enum use_bytecode)
{
	return OpcodeInfo[use_bytecode].name;
}

int GetBytecodeParamsCount(TOpcode::Enum use_bytecode)
{
	return OpcodeInfo[use_bytecode].count;
}

bool GetBytecodeParamExists(TOpcode::Enum use_bytecode, int i)
{
	switch (i)
	{
	case 0:return OpcodeInfo[use_bytecode].f1;
	case 1:return OpcodeInfo[use_bytecode].f2;
	case 2:return OpcodeInfo[use_bytecode].v1;
	case 3:return OpcodeInfo[use_bytecode].v2;
	default:assert(false); return false;
	}
}

bool ValidateOpParams(TOpcode::Enum op, bool f1, bool f2, bool v1, bool v2)
{
	return
		OpcodeInfo[op].f1 == f1&&
		OpcodeInfo[op].f2 == f2&&
		OpcodeInfo[op].v1 == v1&&
		OpcodeInfo[op].v2 == v2;
}

TArrayClassMethod TVirtualMachine::GetArrayClassMethod(int id)
{
	return program->array_class_methods[id];
}

void TVirtualMachine::ConstructStaticVars()
{
	sp = sp + program->static_vars_size;
	memset(sp_first, 0, program->static_vars_size * 4);
	Execute(program->static_vars_init, 0, 0);
}
void TVirtualMachine::DestructStaticVars()
{
	Execute(program->static_vars_destroy, 0, 0);
	sp = sp - program->static_vars_size;
	if (sp != &sp_first[-1])assert(false);//где-то в коммандах не очищается стек
}

void TVirtualMachine::Execute(int method_id, int* stack_top, int* this_pointer)
//stack_top: указатель на первый параметр
{
	using namespace std;
	using namespace TOpcode;
	TOp* op;
	TProgram::TMethod* m = &program->methods_table[method_id];
	if (m->pre_event != -1)
	{
		assert(!m->is_static);
		*(++sp) = (int)this_pointer;
		Execute(m->pre_event, NULL, this_pointer);
	}
	if (m->is_external)
	{
		assert(m->extern_method != NULL);//не забываем задать колбэки для внешних методов
		((TExternalMethod)m->extern_method)(sp, stack_top, this_pointer);
		//чистим стек
		//деструкторы естественно надо вызывать вручную
		//возвращаемое значение
		int par = m->params_size + !m->is_static;
		if (m->return_size != 0)
		{
			memcpy(&sp[1 - par - m->return_size], &sp[1 - m->return_size], m->return_size * 4);
		}
		sp -= par;
		if (m->post_event != -1)//TODO одно и то же в нескольких местах и в RETURN 
		{
			*(++sp) = (int)this_pointer;
			Execute(m->post_event, NULL, this_pointer);
		}
		return;
	}
	op = &program->instructions[m->first_op];
	while (true)
	{
		assert(sp - sp_first <= stack_size);
		//TODO контроль размера стека 
		Execute(op, this_pointer, stack_top, m);
	}
}

void TVirtualMachine::Execute(TOp* op, int* stack_top, int* this_pointer, TProgram::TMethod* m)
{
	using namespace TOpcode;
	switch (op->type)
	{
	case METHOD_HAS_NOT_RETURN_A_VALUE:
		assert(false);
		break;

		//////////////////////////////////////////////////
		// ветвления
	case LABEL:
		assert(false);//должны удалиться при оптимизации
		break;
	case GOTRUE:
		if (*(sp--)){
			op = &program->instructions[op->v1];
			return;
		}
		break;
	case GOFALSE:
		if (!*(bool*)(sp--)){
			op = &program->instructions[op->v1];
			return;
		}
		break;
	case GOTO:
		op = &program->instructions[op->v1];
		return;

		//////////////////////////////////////////////////
		// помещение в стек значений, ссылок и т.д.
	case PUSH:
		*(++sp) = op->v1; break;
	case PUSH_GLOBAL_REF:
		*(++sp) = (int)&Get(op->v1); break;
	case GLOBAL_TESTANDSET:
		if (!Get(op->v2))
		{
			Get(op->v2) = 1;
			//*(++sp)=(int)&Get(op->v2);
		}
		else
		{
			op = &program->instructions[op->v1];
			return;
		}
		break;
	case PUSH_LOCAL_REF:
		*(++sp) = (int)&stack_top[op->v1]; break;
	case PUSH_LOCAL_REF_COPY:
		*(++sp) = stack_top[op->v1]; break;
	case PUSH_MEMBER_REF:
		*(++sp) = (int)&this_pointer[op->v1]; break;
	case PUSH_COUNT:
		sp += op->v1; break;
	case POP_COUNT:
		sp -= op->v1; break;
	case PUSH_THIS:
		*(++sp) = (int)this_pointer; break;
	//case PUSH_STRING_CONST:
	//	sp += 2;
	//	((TString*)(sp - 1))->CopyFromConst(program->string_consts[op->v1]);
	//	break;
	case RVALUE:
	{
		int *s, *d;
		s = (int*)*sp;
		d = sp;
		sp += op->v1 - 1;
		if (op->v2 != -1){
			*(void**)(++sp) = NULL;
			*(void**)(++sp) = s;
			Execute(op->v2, sp, d);
		}
		else
			memcpy(d, s, op->v1 * 4);
	}break;
	case GET_MEMBER:
	{
		int *s, *d;
		int struct_size = sp[-2];
		int member_offset = sp[-1];
		int member_size = sp[0];
		int copy_constr = op->v1;
		int destr = op->v2;
		//copy_constr
		s = sp - struct_size - 2 + member_offset;
		d = sp + 1;
		sp += member_size;
		if (copy_constr != -1){
			*(void**)(++sp) = NULL;
			*(void**)(++sp) = s;
			Execute(copy_constr, sp, d);
		}
		else
			memcpy(d, s, member_size * 4);
		//destr
		if (destr != -1)
		{
			s = sp - struct_size - 2;
			*(void**)(++sp) = NULL;
			Execute(destr, sp + 1, s);
		}
		//
		s = sp - member_size + 1;
		d = sp - struct_size - member_size - 2;
		sp = d + member_size - 1;
		memcpy(d, s, member_size * 4);
	}
		break;

	case ADD_OFFSET:
		*sp += (unsigned int)op->v1 * 4; break;
	case PUSH_STACK_HIGH_REF:
		*(++sp) = (int)&sp[-op->v1];
		break;
	case CALL_METHOD:
	{
		TProgram::TMethod* m_call = &program->methods_table[op->v1];
		Execute(op->v1, (sp + 1 - m_call->params_size), m_call->is_static ? NULL : (int*)sp[-m_call->params_size]);
	}
		break;
	case RETURN:
		if (op->v1)memcpy(&sp[1 - op->v1 - op->v2], &sp[1 - op->v2], op->v2 * 4);
		sp -= op->v1;
		if (m->post_event != -1)
		{
			*(++sp) = (int)this_pointer;
			Execute(m->post_event, NULL, this_pointer);
		}
		//TODO для надежности следует заполнять неиспользуемую память 0xfeefee
		return;

		//////////////////////////////////////////////////
		//static array

	//case R_STATIC_ARR_DEF_CONSTR:
	//	((TStaticArr*)this_pointer)->DefConstr(this, op->v1);
	//	break;
	//case RR_STATIC_ARR_COPY_CONSTR:
	//	((TStaticArr*)this_pointer)->RCopyConstr(this, sp);
	//	break;
	//case R_STATIC_ARR_DESTR:
	//	((TStaticArr*)this_pointer)->Destr(this);
	//	break;
	//case RV_STATIC_ARR_GET_ELEMENT:
	//{
	//	int temp = *(sp--);
	//	TStaticArr* s_arr = (TStaticArr*)*sp;
	//	if (temp > s_arr->methods->el_count - 1 || temp < -1)throw "Ошибка доступа к элементу массива!";
	//	*sp = (int)&s_arr->data[temp*s_arr->methods->el_size];
	//	break;
	//}
	//case STATIC_ARR_ASSIGN:
	//{
	//	int *s, *d;
	//	s = sp - (unsigned int)(op->f1 ? 0 : (op->v1 - 1));
	//	d = s - 1;
	//	((TStaticArr*)*d)->AssignOp(this, (TStaticArr*)(op->f1 ? (int*)*s : s));
	//	if (!op->f1)((TStaticArr*)s)->Destr(this);
	//}break;
	//case STATIC_ARR_EQUAL:
	//{
	//	int *s, *d;
	//	s = sp - (unsigned int)(op->f2 ? 0 : (op->v1 - 1));
	//	d = s - (unsigned int)(op->f1 ? 1 : op->v1);
	//	int temp = ((TStaticArr*)(op->f1 ? (int*)*d : d))->EqualOp(this, (TStaticArr*)(op->f2 ? (int*)*s : s));
	//	*(++sp) = temp;
	//	if (!op->f1)((TStaticArr*)d)->Destr(this);
	//	if (!op->f2)((TStaticArr*)s)->Destr(this);
	//}break;
	//////////////////////////////////////////////////
	//dynamic array

	//case R_DYN_ARR_DEF_CONSTR:
	//	((TDynArr*)this_pointer)->DefConstr(this, op->v1);
	//	break;
	//case RR_DYN_ARR_COPY_CONSTR:
	//	((TDynArr*)this_pointer)->CopyConstr(this, (TDynArr*)*sp);
	//	break;
	//case R_DYN_ARR_DESTR:
	//	((TDynArr*)this_pointer)->Destr(this);
	//	break;
	//case RV_DYN_ARR_GET_ELEMENT:
	//{
	//	int temp = *(sp--);
	//	TDynArr* d_arr = (TDynArr*)*sp;
	//	if (temp > d_arr->v.GetHigh() || temp < -1)throw "Ошибка доступа к элементу массива!";//TODO вывод инфы где и что
	//	*sp = (int)&d_arr->v[temp*d_arr->el_size];
	//	break;
	//}
	//case DYN_ARR_ASSIGN:
	//{
	//	int *s, *d;
	//	s = sp - (unsigned int)(op->f1 ? 0 : (_INTSIZEOF(TDynArr) / 4 - 1));
	//	d = s - 1;
	//	((TDynArr*)*d)->AssignOp(this, (TDynArr*)(op->f1 ? (int*)*s : s));
	//	if (!op->f1)((TDynArr*)s)->Destr(this);
	//}break;

	//case DYN_ARR_EQUAL:
	//{
	//	int *s, *d;
	//	s = sp - (unsigned int)(op->f2 ? 0 : (_INTSIZEOF(TDynArr) / 4 - 1));
	//	d = s - (unsigned int)(op->f1 ? 1 : (_INTSIZEOF(TDynArr) / 4));
	//	int temp = ((TDynArr*)(op->f1 ? (int*)*d : d))->EqualOp(this, (TDynArr*)(op->f2 ? (int*)*s : s));
	//	*(++sp) = temp;
	//	if (!op->f1)((TDynArr*)d)->Destr(this);
	//	if (!op->f2)((TDynArr*)s)->Destr(this);
	//}break;
	//case RV_DYN_ARR_SET_HIGH:
	//	((TDynArr*)(sp[-1]))->SetHigh(this, sp[0]);
	//	sp -= 2;
	//	break;

	//case R_DYN_ARR_GET_HIGH:
	//	*(sp) = (*(TDynArr**)sp)->v.GetHigh();
	//	break;

		//////////////////////////////////////////////////
		//string

	//case R_STRING_DEF_CONSTR:
	//	((TString*)this_pointer)->Init();
	//	break;
	//case RR_STRING_COPY_CONSTR:
	//	((TString*)this_pointer)->Copy((TString*)*sp);
	//	break;
	//case R_STRING_DESTR:
	//	((TString*)this_pointer)->Destr();
	//	break;
	//case STRING_ASSIGN:
	//{
	//	int *s, *d;
	//	s = sp - (unsigned int)(op->f2 ? 0 : (_INTSIZEOF(TString) / 4 - 1));
	//	d = s - 1;
	//	((TString*)(op->f1 ? (int*)*d : d))->AssignOp((TString*)(op->f2 ? (int*)*s : s));
	//	sp = d - 1;
	//	if (!op->f2)((TString*)s)->Destr();
	//}break;
	////case VV_STRING_PLUS:
	////	break;
	////case RV_STRING_PLUS_ASSIGN:
	////	break;
	//case STRING_EQUAL:
	//{
	//	int *s, *d;
	//	s = sp - (unsigned int)(op->f2 ? 0 : (_INTSIZEOF(TString) / 4 - 1));//TODO заменить все sizeof()/4 на _INT_SIZE
	//	d = s - (unsigned int)(op->f1 ? 1 : (_INTSIZEOF(TString) / 4));
	//	int temp = ((TString*)(op->f1 ? (int*)*d : d))->EqualOp((TString*)(op->f2 ? (int*)*s : s));
	//	sp = d;
	//	*sp = temp;
	//	if (!op->f1)((TString*)d)->Destr();
	//	if (!op->f2)((TString*)s)->Destr();
	//}break;
	//case R_STRING_PRINT:
	//	//CharToOem(((TString*)sp[0])->chars,buf);
	//	//printf(buf);
	//	printf(((TString*)sp[0])->chars);
	//	sp--;
	//	break;
	//case V_STRING_PRINT:
	//	//CharToOem(((TString*)(sp-1))->chars,buf);
	//	printf(buf);
	//	printf(((TString*)(sp - 1))->chars);
	//	((TString*)(sp - 1))->Destr();
	//	sp -= 2;
	//	break;




		
	default:
		assert(0);
	}
	op++;
}
bool TVirtualMachine::ExecuteBaseOps(TOp* op, int*& sp, int* object)
{
	using namespace TOpcode;
	//////////////////////////////////////////////////
	//int
	switch (op->type)
	{
	case ASSIGN:
	{
		int *s, *d;
		if (op->f1){
			s = (int*)(sp[0]);
			d = (int*)(sp[-1]);
		}
		else{
			s = &sp[1 - op->v1];
			d = (int*)(sp[-op->v1]);
		}
		memcpy(d, s, op->v1 * 4);
		if (op->f1)
			sp -= 2;
		else
			sp -= op->v1 + 1;
	}break;
	default:
		return false;
	}
	return true;
}
bool TVirtualMachine::ExecuteVec2Ops(TOp* op, int*& sp, int* object)
{
	using namespace TOpcode;
	//////////////////////////////////////////////////
	//int
	switch (op->type)
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
		*(sp - 1) = *(float*)((int*)(sp[-1]) + *sp);
		sp--; break;

	case VV_VEC2_GET_ELEMENT:
		if (*sp != 0 && *sp != 1)throw "Ошибка доступа к элементу вектора!";
		*(sp - 2) = sp[*sp - 2];
		sp -= 2; break;

	case VEC2_DISTANCE:
		*(float*)(sp - 3) = ((TVec2*)(sp - 3))->Distance(*(TVec2*)(sp - 1));
		sp -= 3; break;
	case VEC2_DOT:
		*(float*)(sp - 3) = ((TVec2*)(sp - 3))->AbsScalarMul(*(TVec2*)(sp - 1));
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
bool TVirtualMachine::ExecuteVec2iOps(TOp* op, int*& sp, int* object)
{
	using namespace TOpcode;
	//////////////////////////////////////////////////
	//int
	switch (op->type)
	{
	case VEC2I_CONSTR:
		*(TVec2i*)object = *(TVec2i*)(sp - 1);
		sp-=2; break;
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
bool TVirtualMachine::ExecuteBoolOps(TOp* op, int*& sp, int* object)
{
	using namespace TOpcode;
	//////////////////////////////////////////////////
	//int
	switch (op->type)
	{
	case BOOL_CONSTR:
		*object = sp[0];
		sp--; break;
		//////////////////////////////////////////////////
		//bool
	case BOOL_AND:
		sp[-1] = sp[-1] && sp[0];
		sp--; break;
	case BOOL_OR:
		sp[-1] = sp[-1] || sp[0];
		sp--; break;
	case BOOL_NOT:
		sp[0] = !sp[0]; break;
	case EQUAL:
		Compare(op,sp);
		break;
	case NOT_EQUAL:
		Compare(op,sp);
		sp[0] = (!sp[0]);
		break;
	default:
		return false;
	}
	return true;
}
bool TVirtualMachine::ExecuteFloatOps(TOp* op, int*& sp,int* object)
{
	using namespace TOpcode;
	//////////////////////////////////////////////////
	//int
	switch (op->type)
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
		*(float*)(sp - 2) = Clamp(*(float*)(sp - 2), *(float*)(sp - 1), *(float*)(sp));
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
		*(float*)(++sp) = Randf(); break;
	case FLOAT_SIGN:
		*(float*)sp = (*(float*)sp) > 0.0f ? 1.0f : ((*(float*)sp) == 0.0f ?0.0f: -1.0f); break;
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

bool TVirtualMachine::ExecuteIntOps(TOp* op, int*& sp, int* object)
{
	using namespace TOpcode;
	//////////////////////////////////////////////////
	//int
	switch (op->type)
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
		*sp = *sp > 0 ? 1 : (*sp == 0 ? 0:-1); break;
	default:
		return false;
	}
	return true;
}
