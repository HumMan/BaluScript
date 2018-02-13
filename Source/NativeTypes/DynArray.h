#pragma once

#include "../SemanticInterface/SemanticTreeApi.h"

#include "../TreeRunner/RunContext.h"

#include <stdlib.h>

class TSyntaxAnalyzer;

class TDynArr
{
private:
	TDynArr()
	{
	}
	TDynArr(const TDynArr& r)
	{
	}
	~TDynArr()
	{
	}
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

	void def_constr(TMethodRunContext* run_context);
	void copy_constr(TMethodRunContext* run_context, TDynArr* copy_from);
	void destructor(TMethodRunContext* run_context);
	static void operator_Assign(TMethodRunContext* run_context, TDynArr* left, TDynArr* right);
	static void* operator_GetArrayElement(TMethodRunContext* run_context, TDynArr* obj, int index);

	void resize(TMethodRunContext* run_context, int new_size);
	int size(TMethodRunContext* run_context);

	static SemanticApi::TExternalClassDecl DeclareExternalClass();
};
