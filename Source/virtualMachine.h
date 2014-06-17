#pragma once

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

using namespace std;

typedef void(*TExternalMethod)(int*&,int*,int*);// параметры: вершина стека, указатель на первый параметр, указатель на объект

extern bool ValidateOpParams(TOpcode::Enum op, bool f1,bool f2,bool v1,bool v2);
extern char* GetBytecodeString(TOpcode::Enum use_bytecode);
extern int GetBytecodeParamsCount(TOpcode::Enum use_bytecode);
extern bool GetBytecodeParamExists(TOpcode::Enum use_bytecode,int i);

struct TOp
{
private:
	void Init()
	{
		type=TOpcode::NONE;
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

struct TProgram
{
	struct TMethod
	{
		bool is_static;
		bool is_external;
		int first_op;
		TExternalMethod extern_method;
		int pre_event;
		int post_event;
		int params_size;
		int return_size;//используется для внешних методов, для USER методов все есть в RETURN
		TMethod():first_op(-1),extern_method(NULL){}
	};
	TVector<TOp> instructions;//TODO в дальнейшем в TVector<int>
	struct TArrayClassMethod
	{
		int el_size,el_count;
		//идентификаторы методов и операторов используемых при работе массивов
		int el_def_constr;
		int el_copy_constr;
		int el_destr;
		int el_assign_op[2];//[right_ref]
		int el_equal_op[2][2]; //[left_ref][right_ref]
		bool operator==(TArrayClassMethod& right)
		{
			return memcmp(this,&right,sizeof(TArrayClassMethod))==0;
		}
	};
	TVector<TArrayClassMethod> array_class_methods;
	TVector<std::string> string_consts;
	TVector<TMethod> methods_table;
	int static_vars_init;
	int static_vars_destroy;
	int static_vars_size;
	void SetExternalMethod(int method_id,TExternalMethod external_func)
	{
		if(!methods_table[method_id].is_external)
			throw "Данный метод не является внешним!";
		methods_table[method_id].extern_method=external_func;
	}
};

class TVirtualMachine
{
private:
	int* sp_first;
	int stack_size;
	int* sp;
	TProgram* program;
public:
	int& Get(int id)
	{
		assert(id>=0&&id<stack_size);
		return sp_first[id];
	}
	int* GetHigh()
	{
		return sp;
	}
	void SetProgram(TProgram* p)
	{
		program=p;
	}
	TVirtualMachine(int stack_max_size)
	{
		sp_first=new int[stack_max_size];
		stack_size=stack_max_size;
		sp=&sp_first[-1];
		program=NULL;
	}
	~TVirtualMachine()
	{
		if(sp!=&sp_first[-1])assert(false);//где-то в коммандах не очищается стек
		delete[] sp_first;
	}
	void ConstructStaticVars()
	{
		sp=sp+program->static_vars_size;
		memset(sp_first,0,program->static_vars_size*4);
		Execute(program->static_vars_init,0,0);
	}
	void DestructStaticVars()
	{
		Execute(program->static_vars_destroy,0,0);
		sp=sp-program->static_vars_size;
		if(sp!=&sp_first[-1])assert(false);//где-то в коммандах не очищается стек
	}
	void Push(int v)
	{
		*(++sp)=v;
	}

	struct TVec2
	{
		union
		{
			struct{
				float x,y;
			};
			float v[2];
		};
		TVec2(float v):x(v),y(v){}
		TVec2(float use_x,float use_y):x(use_x),y(use_y){}
		void operator +=(TVec2 v){
			x+=v.x;
			y+=v.y;
		}
		void operator -=(TVec2 v){
			x-=v.x;
			y-=v.y;
		}
		void operator *=(TVec2 v){
			x*=v.x;
			y*=v.y;
		}
		void operator /=(TVec2 v){
			x/=v.x;
			y/=v.y;
		}
		TVec2 operator -()const{
			return TVec2(-x,-y);
		}
		float Cross(TVec2 v)const{
			return x*v.y-y*v.x;
		}
		float Distance(TVec2 v)const{
			return TVec2(x-v.x,y-v.y).Length();
		}
		float Dot(TVec2 v)const{
			return x*v.x+y*v.y;
		}
		float Length()const{
			return sqrt(x*x+y*y);
		}
		void Normalize(){
			(*this)/=sqrt(x*x+y*y);
		}
		void Reflect(TVec2 v){
			float t=(x*v.x+y*v.y)*2.0f;
			x-=v.x*t;
			y-=v.y*t;
		}
		void Inverse(){
			x=-x;
			y=-y;
		}
	};
private:
	void ArrayElementsDefConstr(int* v, int el_size, int el_count, int el_def_constr, int first_el=0)
	{
		for(int i=first_el*el_size;i<(first_el+el_count)*el_size;i+=el_size)
		{
			*(void**)(++sp)=NULL;
			Execute(el_def_constr,NULL,&v[i]);
		}
	}
	void ArrayElementsRCopyConstr(int* copy_to,int* copy_from, int el_size, int el_count, int el_copy_constr)
	{
		for(int i=0;i<el_count*el_size;i+=el_size)
		{
			*(void**)(++sp)=NULL;
			*(void**)(++sp)=&copy_from[i];
			Execute(el_copy_constr,sp,&copy_to[i]);
		}
	}
	void ArrayElementsDestr(int* v, int el_size, int el_count, int el_destr, int first_el=0)
	{
		for(int i=first_el*el_size;i<(first_el+el_count)*el_size;i+=el_size)
		{
			*(void**)(++sp)=NULL;
			Execute(el_destr,NULL,&v[i]);
		}
	}

	void ArrayElementsRRAssignOp(int* copy_to,int* copy_from, int el_size, int el_count, int el_assign_op)
	{
		for(int i=0;i<el_count*el_size;i+=el_size)
		{
			*(void**)(++sp)=&copy_to[i];
			*(void**)(++sp)=&copy_from[i];
			Execute(el_assign_op,sp-1,NULL);
		}
	}

	void ArrayElementsRVAssignOp(int* copy_to,int* copy_from, int el_size, int el_count, int el_assign_op, int el_copy_constr)
	{
		for(int i=0;i<el_count*el_size;i+=el_size)
		{
			int* first_par=sp+1;
			*(++sp)=(int)&copy_to[i];
			int* second_par=sp+1;
			sp+=el_size;
			//вызываем конструктор копии для второго параметра
			*(void**)(++sp)=NULL;
			*(void**)(++sp)=&copy_from[i];
			Execute(el_copy_constr,NULL,second_par);
			//
			Execute(el_assign_op,first_par,NULL);
		}
	}

	bool ArrayElementsRREqualOp(int* left,int* right, int el_size, int el_count, int el_equal_op)
	{
		for(int i=0;i<el_count*el_size;i+=el_size)
		{
			*(void**)(++sp)=&left[i];
			*(void**)(++sp)=&right[i];
			Execute(el_equal_op,sp-1,NULL);
			if(*sp==0){sp--;return false;}
			sp--;
		}
		return true;
	}

	bool ArrayElementsVVEqualOp(int* left,int* right, int el_size, int el_count, int el_equal_op, int el_copy_constr)
	{
		for(int i=0;i<el_count*el_size;i+=el_size)
		{
			int* first_par=sp+1;
			*(void**)(++sp)=&left[i];
			int* second_par=sp+1;
			sp+=el_size;
			//вызываем конструктор копии для второго параметра
			*(void**)(++sp)=NULL;
			*(void**)(++sp)=&right[i];
			Execute(el_copy_constr,sp,second_par);//TODO что-то непонятно а где конструктор копии для первого параметра
			//
			Execute(el_equal_op,first_par,NULL);
			if(*sp==0){sp--;return false;}
			sp--;
		}
		return true;
	}

public:

	template<int COUNT,class T=int>
	struct TStaticArr
	{
	public:
		TProgram::TArrayClassMethod* methods;
		int data[COUNT*_INTSIZEOF(T)/4];//размер не имеет значения т.к. TStaticArr никогда не создается
		int* Get(int i)
		{
			assert(i<COUNT);
			return &data[i*methods->el_size];
		}
		T& operator[](int id)
		{
			assert(id<COUNT);
			return *(T*)&data[id*_INTSIZEOF(T)/4];
		}
		void DefConstr(TVirtualMachine* machine,int use_methods_id)
		{
			methods=&machine->program->array_class_methods[use_methods_id];
			int el_size=methods->el_size;
			int high=methods->el_count-1;
			int el_def_constr=methods->el_def_constr;
			if(el_def_constr!=-1)
				machine->ArrayElementsDefConstr(&data[0],el_size,high+1,el_def_constr);
		}
		void RCopyConstr(TVirtualMachine* machine, int* &sp)
		{
			TStaticArr* copy_from=(TStaticArr*)*sp;
			methods=copy_from->methods;
			int el_size=methods->el_size;
			int high=methods->el_count-1;

			int el_copy_constr=methods->el_copy_constr;
			if(el_copy_constr!=-1)
				machine->ArrayElementsRCopyConstr(&data[0],&copy_from->data[0],el_size,high+1,el_copy_constr);
			else if(high>-1)memcpy(&data[0],&copy_from->data[0],(high+1)*el_size*4);
		}
		void Destr(TVirtualMachine* machine)
		{
			int el_size=methods->el_size;
			int high=methods->el_count-1;
			int el_destr=methods->el_destr;
			if(el_destr!=-1)
				machine->ArrayElementsDestr(&data[0],el_size,high+1,el_destr);
		}
		void AssignOp(TVirtualMachine* machine, TStaticArr* right)
		{
			int el_size=methods->el_size;
			int high=methods->el_count-1;
			int el_assign_op=methods->el_assign_op[true];
			if(el_assign_op!=-1)
				machine->ArrayElementsRRAssignOp(&data[0],&right->data[0],el_size,high+1,el_assign_op);
			else if(high>-1) memcpy(&data[0],&right->data[0],(high+1)*el_size*4);
		}
		bool EqualOp(TVirtualMachine* machine, TStaticArr* right)
		{
			bool result=true;
			int el_size=methods->el_size;
			int high=methods->el_count-1;
			int el_equal_op=methods->el_equal_op[true][true];//TODO выбор оптимального оператора из доступных
			if(el_equal_op!=-1)
				result=machine->ArrayElementsRREqualOp(&data[0],&right->data[0],el_size,high+1,el_equal_op);
			else if(high>-1) result=(memcmp(&data[0],&right->data[0],(high+1)*el_size*4)==0);
			else result=true;
			return result;
		}
	};

	struct TDynArr
	{
		TVector<int> v;
		int el_size;
		TProgram::TArrayClassMethod* methods;
		int* Get(int i)
		{
			return &v[i*el_size];
		}
		int GetHigh()
		{
			return (v.GetHigh()+1)/el_size-1;
		}
		void DefConstr(TVirtualMachine* machine,int use_methods_id)
		{
			methods=&machine->program->array_class_methods[use_methods_id];
			el_size=methods->el_size;
			v.Init();
		}
		void CopyConstr(TVirtualMachine* machine, TDynArr* copy_from)
		{
			methods=copy_from->methods;
			el_size=copy_from->el_size;
			v.Init();
			SetHigh(machine,copy_from->v.GetCount()/el_size-1,false);
			if(v.GetHigh()>-1)
			{
				int el_copy_constr=methods->el_copy_constr;
				if(el_copy_constr!=-1)
					machine->ArrayElementsRCopyConstr(&v[0],&copy_from->v[0],el_size,v.GetCount()/el_size,el_copy_constr);
				else memcpy(&v[0],&copy_from->v[0],v.GetCount()*sizeof(int));
			}
		}
		void Destr(TVirtualMachine* machine)
		{
			int el_destr=methods->el_destr;
			if(el_destr!=-1&&v.GetHigh()>-1)
				machine->ArrayElementsDestr(&v[0],el_size,((v.GetHigh()+1)/el_size),el_destr);
			v.~TVector();
		}
		void AssignOp(TVirtualMachine* machine, TDynArr* right)
		{
			if(v.GetHigh()!=right->v.GetHigh())
				SetHigh(machine,right->v.GetHigh());
			int el_rr_assign_op=methods->el_assign_op[true];
			int el_rv_assign_op=methods->el_assign_op[false];
			if(v.GetHigh()>-1)
			{
				if(el_rr_assign_op!=-1)
					machine->ArrayElementsRRAssignOp(&v[0],&right->v[0],el_size,(v.GetCount()/el_size),el_rr_assign_op);
				else if(el_rv_assign_op!=-1)
					machine->ArrayElementsRVAssignOp(&v[0],&right->v[0],el_size,(v.GetCount()/el_size),el_rv_assign_op,methods->el_copy_constr);
				else memcpy(&v[0],&right->v[0],v.GetCount()*sizeof(int));
			}
		}
		bool EqualOp(TVirtualMachine* machine, TDynArr* right)
		{
			bool result=true;
			int el_equal_op=methods->el_equal_op[true][true];//TODO выбор оптимального оператора
			if(v.GetHigh()!=right->v.GetHigh())result=false;
			else if(el_equal_op!=-1)
				result=machine->ArrayElementsRREqualOp(&v[0],&right->v[0],el_size,(v.GetCount()/el_size),el_equal_op);
			else if(v.GetHigh()>-1) result=(memcmp(&v[0],&right->v[0],v.GetCount()*sizeof(int))==0);
			else result=true;
			return result;
		}
		void SetHigh(TVirtualMachine* machine,int new_high,bool call_constr_destr=true)
		{
			int high=((v.GetHigh()+1)/el_size)-1;
			if(new_high==high)return;
			if(call_constr_destr)
			{
				int el_destr=methods->el_destr;
				if(el_destr!=-1&&new_high<high)
					machine->ArrayElementsDestr(&v[0],el_size,high-new_high,el_destr,new_high+1);
			}
			v.SetCount((new_high+1)*el_size);
			if(call_constr_destr)
			{
				int el_def_constr=methods->el_def_constr;
				if(el_def_constr!=-1&&new_high>high)
					machine->ArrayElementsDefConstr(&v[0],el_size,new_high-high,el_def_constr,high+1);
			}
		}
		int* IncHigh(TVirtualMachine* machine,int count=1)
		{
			SetHigh(machine,(v.GetHigh()+1)/el_size-1+count);
			return &v[v.GetHigh()-el_size+1];
		}
	};

	struct TString
	{
		char* chars;
		int length;
		//bool is_const;
		//TODO is_const флаг означающий что в данный момент chars указывает на константу(для исключения выделения памяти при работе с литералами)
		void Init(){
			chars=NULL;
			length=0;
			//is_const=false;
		}
		void Copy(TString* copy_from)
		{
			chars=copy_from->length!=0?new char[copy_from->length+1]:NULL;
			length=copy_from->length;
			if (length>0)strncpy_s(chars, copy_from->length + 1, copy_from->chars, copy_from->length + 1);
		}
		void CopyFromConst(std::string& copy_from)
		{
			//is_const=true;
			length=copy_from.length()+1;
			chars=new char[length];
			strncpy_s(chars, length,copy_from.c_str(), length);
		}
		void Destr()
		{
			if(chars!=NULL)delete chars;
		}
		void AssignOp(TString* right)
		{
			if(length!=right->length)
			{
				if(chars!=NULL)delete chars;
				chars=right->length!=0?new char[right->length+1]:NULL;
				length=right->length;
			}
			if(length>0)
				strncpy_s(chars, right->length + 1,right->chars, right->length + 1);
		}
		bool EqualOp(TString* right)
		{
			return strcmp(chars,right->chars)==0;
		}
	};
	void Execute(int method_id)
	{
		Execute(method_id,sp+1,0);
	}
	void Execute(int method_id,int* this_pointer)
	{
		Push((int)this_pointer);
		Execute(method_id,sp+1,this_pointer);
	}
	char buf[10000];
	void Compare(TOp* op)
	{
		int *s,*d;
		if(op->f2)s=&sp[0];
		else s=&sp[1-op->v1];
		if(op->f1) d=(int*)s[-1];
		else d=s-op->v1;
		if(op->f2)s=(int*)*s;
		sp-=(op->f1?1:op->v1)+(op->f2?1:op->v1)-1;
		*sp=(memcmp(d,s,op->v1*4)==0);
	}
	void Execute(int method_id,int* stack_top,int* this_pointer);
};
