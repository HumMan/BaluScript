#pragma once

#include <baluLib.h>

class TStaticValue;
class TStackValue;
class TSyntaxAnalyzer;
class TSClass;

struct TDynArr
{
	std::vector<int>* v;//можно использовать не указатель, но учитывать move конструктор вектора, который хранит backpointer
	TSClass* el_class;
	void Init()
	{
		el_class = NULL;
		v = new std::vector<int>();
	}
	void Init(const TDynArr& r)
	{
		el_class = NULL;
		v = new std::vector<int>();
	}
	TDynArr()
	{
		Init();
	}
	TDynArr(const TDynArr& r)
	{
		Init(r);
	}
	void* GetElement(int i);
	int GetSize();
	void EmplaceBack(std::vector<TStaticValue> &static_fields);
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