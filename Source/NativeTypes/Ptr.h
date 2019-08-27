#pragma once

#include "../SemanticInterface/SemanticTreeApi.h"

#include "../TreeRunner/RunContext.h"

#include <stdlib.h>

class TSyntaxAnalyzer;

class TPtr
{
public:
	void* v;
	int* refs_count;
	SemanticApi::ISClass* data_class;
	void Init()
	{
		data_class = nullptr;
		v = nullptr;
		refs_count = nullptr;
	}

	void def_constr(TMethodRunContext& run_context);
	void copy_constr(TMethodRunContext& run_context, TPtr* copy_from);
	void destructor(TMethodRunContext& run_context);
	static void operator_Assign(TMethodRunContext& run_context, TPtr* left, TPtr* right);

	void* data(TMethodRunContext& run_context);

	static SemanticApi::TExternalClassDecl DeclareExternalClass();
};
