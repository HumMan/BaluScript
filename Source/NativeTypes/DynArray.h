#pragma once

#include <baluLib.h>
//#include "../VirtualMachine/Program.h"
//#include "../VirtualMachine.h"
//
//class TArrayClassMethod;
//struct TDynArr
//{
//	BaluLib::TVector<int> v;
//	int el_size;
//	TArrayClassMethod* methods;
//	int* Get(int i);
//	int GetHigh();
//	void DefConstr(TVirtualMachine* machine, int use_methods_id);
//	void CopyConstr(TVirtualMachine* machine, TDynArr* copy_from);
//	void Destr(TVirtualMachine* machine);
//	void AssignOp(TVirtualMachine* machine, TDynArr* right);
//	bool EqualOp(TVirtualMachine* machine, TDynArr* right);
//	void SetHigh(TVirtualMachine* machine, int new_high, bool call_constr_destr = true);
//	int* IncHigh(TVirtualMachine* machine, int count = 1);
//};

class TStaticValue;
class TStackValue;
class TSyntaxAnalyzer;
class TSClass;

struct TDynArr
{
	std::vector<int>* v;//можно использовать не указатель, но учитывать move конструктор вектора, который видимо хранит backpointer
	TSClass* el_class;
	TDynArr()
	{
		el_class = NULL;
		v = new std::vector<int>();
	}
	TDynArr(const TDynArr& r)
	{
		el_class = r.el_class;
		v = new std::vector<int>(*r.v);
	}
	~TDynArr()
	{
		el_class = NULL;
		delete v;
		v = NULL;
	}
	static void constructor(std::vector<TStaticValue> &static_fields, std::vector<TStackValue> &formal_params, TStackValue& result, TStackValue& object);
	static void destructor(std::vector<TStaticValue> &static_fields, std::vector<TStackValue> &formal_params, TStackValue& result, TStackValue& object);
	static void copy_constr(std::vector<TStaticValue> &static_fields, std::vector<TStackValue> &formal_params, TStackValue& result, TStackValue& object);
	static void assign_op(std::vector<TStaticValue> &static_fields, std::vector<TStackValue> &formal_params, TStackValue& result, TStackValue& object);
	static void get_element_op(std::vector<TStaticValue> &static_fields, std::vector<TStackValue> &formal_params, TStackValue& result, TStackValue& object);
	static void resize(std::vector<TStaticValue> &static_fields, std::vector<TStackValue> &formal_params, TStackValue& result, TStackValue& object);
	static void get_size(std::vector<TStaticValue> &static_fields, std::vector<TStackValue> &formal_params, TStackValue& result, TStackValue& object);

	static void DeclareExternalClass(TSyntaxAnalyzer* syntax);
};