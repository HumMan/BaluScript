﻿#include "Ptr.h"

#include "../TreeRunner/FormalParam.h"

#include "../syntaxAnalyzer.h"
#include "../SemanticInterface/SemanticTreeApi.h"

#include <string.h>

#include "../TreeRunner/TreeRunner.h"

void TPtr::def_constr(TMethodRunContext& run_context)
{
	Init();
	data_class = run_context.GetObject().GetClass()->GetTemplateParam(0).GetType();
	v = new int[data_class->GetSize()];
	refs_count = new int(1);
	SemanticApi::ISMethod* el_def_constr = data_class->GetDefConstr();
	if (el_def_constr != nullptr)
	{
		TStackValue el_obj(true, data_class);
		el_obj.SetAsReference(v);

		TMethodRunContext method_context(run_context.GetGlobalContext());
		method_context.GetObject() = std::move(el_obj);

		TreeRunner::Run(el_def_constr, method_context);
	}
}

void TPtr::destructor(TMethodRunContext& run_context)
{
	(*refs_count)--;
	if (*refs_count == 0)
	{
		SemanticApi::ISMethod* el_destr = data_class->GetDestructor();
		if (el_destr != nullptr)
		{
			TStackValue el_obj(true, data_class);
			el_obj.SetAsReference(v);

			TMethodRunContext method_context(run_context.GetGlobalContext());
			method_context.GetObject() = std::move(el_obj);

			TreeRunner::Run(el_destr, method_context);
		}
		delete v;
		delete refs_count;
	}

	refs_count = nullptr;	
	v = nullptr;
	if(run_context.GetObject().get()!=nullptr)
		memset((TPtr*)run_context.GetObject().get(), 0xfeefee, sizeof(TPtr));
}

void TPtr::copy_constr(TMethodRunContext& run_context, TPtr* copy_from)
{
	v = copy_from->v;
	refs_count = copy_from->refs_count;
	(*refs_count)++;
}

void TPtr::operator_Assign(TMethodRunContext& run_context, TPtr* left, TPtr* right)
{
	if (left->v == right->v)
		return;
	left->destructor(run_context);

	left->v = right->v;
	left->refs_count = right->refs_count;
	(*left->refs_count)++;
}

void* TPtr::data(TMethodRunContext& run_context)
{
	return v;
}

SemanticApi::TExternalClassDecl TPtr::DeclareExternalClass()
{
	SemanticApi::TExternalClassDecl decl;

	decl.source=
		"class extern TPtr<T>\n"
		"{\n"
		"default();\n"
		"copy(TPtr& copy_from);\n"
		"destr();\n"
		"operator static =(TPtr& v,TPtr& l);\n"
		"func data:&T;\n"
		"}\n";


	decl.size = LexerIntSizeOf(sizeof(TPtr)) / sizeof(int);
	return decl;
}
