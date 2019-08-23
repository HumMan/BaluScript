#include "DynArray.h"

#include "../TreeRunner/FormalParam.h"

#include "../syntaxAnalyzer.h"
#include "../SemanticInterface/SemanticTreeApi.h"

#include <string.h>

#include "../TreeRunner/TreeRunner.h"

void TDynArr::def_constr(TMethodRunContext* run_context)
{
	Init();
	el_class = run_context->object->GetClass()->GetTemplateParam(0).GetType();
}

void CallMethod(TGlobalRunContext global_context, int* v, int first_element, int el_count, int el_size, SemanticApi::ISClass* el_class, SemanticApi::ISMethod* method)
{
	for (size_t i = first_element*el_size; i<el_count*el_size; i += el_size)
	{
		TStackValue el_obj(true, el_class);
		std::vector<TStackValue> without_params;
		TStackValue without_result;
		
		el_obj.SetAsReference(&v[i]);

		TreeRunner::Run(method, TMethodRunContext(global_context, &without_params, &without_result, &el_obj));
	}
}

void CallCopyConstr(TGlobalRunContext global_context, int* v, int* copy_from, int first_element, int el_count, int el_size, SemanticApi::ISClass* el_class, SemanticApi::ISMethod* method)
{
	for (size_t i = first_element*el_size; i<el_count*el_size; i += el_size)
	{
		TStackValue el_obj(true, el_class);
		el_obj.SetAsReference(&v[i]);

		std::vector<TStackValue> params;
		TStackValue without_result;
		params.emplace_back(true, el_class);
		params[0].SetAsReference(&copy_from[i]);
		
		TreeRunner::Run(method, TMethodRunContext(global_context, &params, &without_result, &el_obj));
	}
}

void CallAssignOp(TGlobalRunContext global_context, int* left, int* right, int first_element, int el_count, int el_size, SemanticApi::ISClass* el_class, SemanticApi::ISMethod* method)
{
	for (size_t i = first_element*el_size; i<el_count*el_size; i += el_size)
	{
		TStackValue el_obj;
		std::vector<TStackValue> params;
		TStackValue without_result;
		params.emplace_back(true, el_class);
		params[0].SetAsReference(&left[i]);
		params.emplace_back(true, el_class);
		params[1].SetAsReference(&right[i]);
		TreeRunner::Run(method, TMethodRunContext(global_context, &params, &without_result, &el_obj));
	}
}

void CheckRefs(TMethodRunContext* run_context, void* left, size_t count)
{
	if (run_context->refs_list->RefsInRange(left, (char*)left + count * sizeof(int)))
	{
		throw RuntimeException(RuntimeExceptionId::DynArray_UserAfterFree);
	}
}

void dyn_arr_resize(TGlobalRunContext global_context, TDynArr* obj, int new_size)
{
	SemanticApi::ISClass* el = obj->el_class;
	size_t curr_size = obj->v->size() / el->GetSize();

	if (curr_size == new_size)
		return;

	if (curr_size > new_size)
	{
		SemanticApi::ISMethod* el_destr = el->GetDestructor();
		if (el_destr != nullptr)
		{
			CallMethod(global_context, &((*obj->v)[0]), new_size, curr_size, el->GetSize(), el, el_destr);
		}
	}	
	obj->v->resize(el->GetSize()*new_size);
	if (curr_size < new_size)
	{
		SemanticApi::ISMethod* el_def_constr = el->GetDefConstr();
		if (el_def_constr != nullptr)
		{
			CallMethod(global_context, &((*obj->v)[0]), curr_size, new_size, el->GetSize(), el, el_def_constr);
		}
	}
}

void TDynArr::destructor(TMethodRunContext* run_context)
{
	if(v->size()>0)
		CheckRefs(run_context, &(*v)[0], v->size());

	SemanticApi::ISMethod* el_destr = el_class->GetDestructor();
	if (el_destr != nullptr)
	{
		if (v->size() > 0)
		{
			CallMethod(*run_context, &((*v)[0]), 0, v->size() / el_class->GetSize(), el_class->GetSize(), el_class, el_destr);
		}
	}

	el_class = nullptr;
	delete v;
	v = nullptr;

	memset((TDynArr*)run_context->object->get(), 0xfeefee, sizeof(TDynArr));
}

void TDynArr::copy_constr(TMethodRunContext* run_context, TDynArr* copy_from)
{
	SemanticApi::ISClass* el = copy_from->el_class;
	SemanticApi::ISMethod* el_copy_constr = el->GetCopyConstr();
	
	if (el_copy_constr != nullptr)
	{
		Init();
		el_class = el;
		if (copy_from->v->size() > 0)
		{
			v->resize(copy_from->v->size());
			CallCopyConstr(*run_context, &((*v)[0]), &((*copy_from->v)[0]), 
				0, copy_from->v->size() / el->GetSize(), el->GetSize(), el, el_copy_constr);
		}
	}
	else
	{
		memset((TDynArr*)run_context->object->get(), 0xfeefee, sizeof(TDynArr));
		Init(*copy_from);
	}
}

void TDynArr::operator_Assign(TMethodRunContext* run_context, TDynArr* left, TDynArr* right)
{
	if (left->v->size() > 0)
		CheckRefs(run_context, &(*left->v)[0], right->v->size());

	std::vector<SemanticApi::ISMethod*> ops;

	left->el_class->GetOperators(ops, Lexer::TOperator::Assign);
	//TODO поиск нужного оператора присваивания
	dyn_arr_resize(*run_context, left, right->v->size() / left->el_class->GetSize());
	if (left->v->size() > 0)
	{
		if(ops.size()>0)
			CallAssignOp(*run_context, &((*left->v)[0]), &((*right->v)[0]), 0, left->v->size() / left->el_class->GetSize(), left->el_class->GetSize(), left->el_class, ops[0]);
		else
		{
			memcpy(&((*left->v)[0]), &((*right->v)[0]), left->v->size()*sizeof(int));
		}
	}
}

void* TDynArr::operator_GetArrayElement(TMethodRunContext* run_context, TDynArr* obj, int index)
{
	SemanticApi::ISClass* el = obj->el_class;
	return &(*obj->v)[el->GetSize()*index];
}

void TDynArr::resize(TMethodRunContext* run_context, int new_size)
{
	if (v->size() > 0)
		CheckRefs(run_context, &(*v)[0], v->size());

	dyn_arr_resize(*run_context, this, new_size);
}

int TDynArr::size(TMethodRunContext* run_context)
{
	return v->size() / el_class->GetSize();
}

SemanticApi::TExternalClassDecl TDynArr::DeclareExternalClass()
{
	SemanticApi::TExternalClassDecl decl;

	decl.source=
		"class extern TDynArray<T>\n"
		"{\n"
		"default();\n"
		"copy(TDynArray& copy_from);\n"
		"destr();\n"
		"operator static [](TDynArray& v,int id):&T;\n"
		"operator static =(TDynArray& v,TDynArray& l);\n"
		"func resize(int new_size);\n"
		"func size:int;\n"
		"}\n";


	decl.size = LexerIntSizeOf(sizeof(TDynArr)) / sizeof(int);
	return decl;
}
