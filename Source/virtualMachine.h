#pragma once

#include <assert.h>
#include <memory.h>
#include <string.h>
#include "VirtualMachine/Op.h"
#include "VirtualMachine/ArrayClassMethod.h"
#include "VirtualMachine/Program.h"

class TProgram;

class TVirtualMachine
{
private:
	int* sp_first;
	int stack_size;
	int* sp;
	TProgram* program;
public:
	TArrayClassMethod GetArrayClassMethod(int id);
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
		program=nullptr;
	}
	~TVirtualMachine()
	{
		if(sp!=&sp_first[-1])assert(false);//где-то в коммандах не очищается стек
		delete[] sp_first;
	}
	void ConstructStaticVars();
	void DestructStaticVars();
	void Push(int v)
	{
		*(++sp)=v;
	}
public:
	void ArrayElementsDefConstr(int* v, int el_size, int el_count, int el_def_constr, int first_el=0)
	{
		for(int i=first_el*el_size;i<(first_el+el_count)*el_size;i+=el_size)
		{
			*(void**)(++sp)=nullptr;
			Execute(el_def_constr,nullptr,&v[i]);
		}
	}
	void ArrayElementsRCopyConstr(int* copy_to,int* copy_from, int el_size, int el_count, int el_copy_constr)
	{
		for(int i=0;i<el_count*el_size;i+=el_size)
		{
			*(void**)(++sp)=nullptr;
			*(void**)(++sp)=&copy_from[i];
			Execute(el_copy_constr,sp,&copy_to[i]);
		}
	}
	void ArrayElementsDestr(int* v, int el_size, int el_count, int el_destr, int first_el=0)
	{
		for(int i=first_el*el_size;i<(first_el+el_count)*el_size;i+=el_size)
		{
			*(void**)(++sp)=nullptr;
			Execute(el_destr,nullptr,&v[i]);
		}
	}

	void ArrayElementsRRAssignOp(int* copy_to,int* copy_from, int el_size, int el_count, int el_assign_op)
	{
		for(int i=0;i<el_count*el_size;i+=el_size)
		{
			*(void**)(++sp)=&copy_to[i];
			*(void**)(++sp)=&copy_from[i];
			Execute(el_assign_op,sp-1,nullptr);
		}
	}

	void ArrayElementsRVAssignOp(int* copy_to,int* copy_from, int el_size, int el_count, int el_assign_op, int el_copy_constr)
	{
		for(int i=0;i<el_count*el_size;i+=el_size)
		{
			int* first_par=sp+1;
			*(++sp)=(intptr_t)&copy_to[i];
			int* second_par=sp+1;
			sp+=el_size;
			//вызываем конструктор копии для второго параметра
			*(void**)(++sp)=nullptr;
			*(void**)(++sp)=&copy_from[i];
			Execute(el_copy_constr,nullptr,second_par);
			//
			Execute(el_assign_op,first_par,nullptr);
		}
	}

	bool ArrayElementsRREqualOp(int* left,int* right, int el_size, int el_count, int el_equal_op)
	{
		for(int i=0;i<el_count*el_size;i+=el_size)
		{
			*(void**)(++sp)=&left[i];
			*(void**)(++sp)=&right[i];
			Execute(el_equal_op,sp-1,nullptr);
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
			*(void**)(++sp)=nullptr;
			*(void**)(++sp)=&right[i];
			Execute(el_copy_constr,sp,second_par);//TODO что-то непонятно а где конструктор копии для первого параметра
			//
			Execute(el_equal_op,first_par,nullptr);
			if(*sp==0){sp--;return false;}
			sp--;
		}
		return true;
	}

public:

	void Execute(int method_id)
	{
		Execute(method_id,sp+1,0);
	}
	void Execute(int method_id,int* this_pointer)
	{
		Push((intptr_t)this_pointer);
		Execute(method_id,sp+1,this_pointer);
	}
	
	void Execute(int method_id,int* stack_top,int* this_pointer);
	void Execute(TOp* op, int* stack_top, int* this_pointer, TProgram::TMethod* m);

};

class TSimpleOps
{
public:
	static bool ExecuteIntOps(const TOp& op, int*& sp, int* object);
	static bool ExecuteFloatOps(const TOp& op, int*& sp, int* object);
	static bool ExecuteBoolOps(const TOp& op, int*& sp, int* object);
	static bool ExecuteVec2Ops(const TOp& op, int*& sp, int* object);
	static bool ExecuteVec2iOps(const TOp& op, int*& sp, int* object);
	static bool ExecuteBaseOps(const TOp& op, int*& sp, int* object);
};