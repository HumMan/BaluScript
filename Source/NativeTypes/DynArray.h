#pragma once

#include "../SemanticInterface/SemanticTreeApi.h"

#include "../TreeRunner/RunContext.h"

#include <stdlib.h>

class TSyntaxAnalyzer;

class TDynArr
{
public:
	std::vector<int>* v;//можно использовать не указатель, но учитывать move конструктор вектора, который хранит backpointer
	SemanticApi::ISClass* el_class;
	void Init()
	{
		el_class = nullptr;
		v = new std::vector<int>();
	}
	void Init(const TDynArr& r)
	{
		el_class = nullptr;
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
		el_class = nullptr;
		delete v;
		v = nullptr;
	}
	static void constructor(TMethodRunContext* run_context);
	static void destructor(TMethodRunContext* run_context);
	static void copy_constr(TMethodRunContext* run_context);
	static void assign_op(TMethodRunContext* run_context);
	static void get_element_op(TMethodRunContext* run_context);
	static void resize(TMethodRunContext* run_context);
	static void get_size(TMethodRunContext* run_context);

	static void DeclareExternalClass(TSyntaxAnalyzer* syntax);
};
