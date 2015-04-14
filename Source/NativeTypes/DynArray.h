#pragma once

#include <baluLib.h>

class TStaticValue;
class TStackValue;
class TSyntaxAnalyzer;
class TSClass;

#include "..\Semantic\RunContext.h"

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
	static void constructor(TMethodRunContext run_context);
	static void destructor(TMethodRunContext run_context);
	static void copy_constr(TMethodRunContext run_context);
	static void assign_op(TMethodRunContext run_context);
	static void get_element_op(TMethodRunContext run_context);
	static void resize(TMethodRunContext run_context);
	static void get_size(TMethodRunContext run_context);

	static void DeclareExternalClass(TSyntaxAnalyzer* syntax);
};