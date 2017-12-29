#include "DynArray.h"

#include "../TreeRunner/FormalParam.h"

#include "../syntaxAnalyzer.h"
#include "../SemanticInterface/SemanticTreeApi.h"

#include <string.h>

void TDynArr::constructor(TMethodRunContext* run_context)
{
	run_context->object->get_as<TDynArr>().Init();
	run_context->object->get_as<TDynArr>().el_class = run_context->object->GetClass()->GetTemplateParam(0).GetType();
}

void CallMethod(std::vector<TStaticValue> &static_fields, int* v, int first_element, int el_count, int el_size, SemanticApi::ISClass* el_class, SemanticApi::ISMethod* method)
{
	for (size_t i = first_element*el_size; i<el_count*el_size; i += el_size)
	{
		TStackValue el_obj(true, el_class);
		std::vector<TStackValue> without_params;
		TStackValue without_result;
		
		el_obj.SetAsReference(&v[i]);

		//method->Run(TMethodRunContext(&static_fields, &without_params, &without_result, &el_obj));
	}
}

void CallCopyConstr(std::vector<TStaticValue> &static_fields, int* v, int* copy_from, int first_element, int el_count, int el_size, SemanticApi::ISClass* el_class, SemanticApi::ISMethod* method)
{
	for (size_t i = first_element*el_size; i<el_count*el_size; i += el_size)
	{
		TStackValue el_obj(true, el_class);
		el_obj.SetAsReference(&v[i]);

		std::vector<TStackValue> params;
		TStackValue without_result;
		params.resize(1);
		params[0] = TStackValue(true, el_class);
		params[0].SetAsReference(&copy_from[i]);
		
		//method->Run(TMethodRunContext(&static_fields, &params, &without_result, &el_obj));
	}
}

void CallAssignOp(std::vector<TStaticValue> &static_fields, int* left, int* right, int first_element, int el_count, int el_size, SemanticApi::ISClass* el_class, SemanticApi::ISMethod* method)
{
	for (size_t i = first_element*el_size; i<el_count*el_size; i += el_size)
	{
		TStackValue el_obj;
		std::vector<TStackValue> params;
		TStackValue without_result;
		params.resize(2);
		params[0] = TStackValue(true, el_class);
		params[0].SetAsReference(&left[i]);
		params[1] = TStackValue(true, el_class);
		params[1].SetAsReference(&right[i]);
		//method->Run(TMethodRunContext(&static_fields, &params, &without_result, &el_obj));
	}
}

void dyn_arr_resize(std::vector<TStaticValue> &static_fields, TDynArr* obj, int new_size)
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
			CallMethod(static_fields, &((*obj->v)[0]), new_size, curr_size, el->GetSize(), el, el_destr);
		}
	}
	obj->v->resize(el->GetSize()*new_size);
	if (curr_size < new_size)
	{
		SemanticApi::ISMethod* el_def_constr = el->GetDefConstr();
		if (el_def_constr != nullptr)
		{
			CallMethod(static_fields, &((*obj->v)[0]), curr_size, new_size, el->GetSize(), el, el_def_constr);
		}
	}
}

void TDynArr::destructor(TMethodRunContext* run_context)
{
	TDynArr* obj = ((TDynArr*)run_context->object->get());
	SemanticApi::ISClass* el = obj->el_class;
	SemanticApi::ISMethod* el_destr = el->GetDestructor();
	if (el_destr != nullptr)
	{
		if (obj->v->size() > 0)
		{
			CallMethod(*run_context->static_fields, &((*obj->v)[0]), 0, obj->v->size() / el->GetSize(), el->GetSize(), el, el_destr);
		}
	}
	obj->~TDynArr();
	memset((TDynArr*)run_context->object->get(), 0xfeefee, sizeof(TDynArr));
}

void TDynArr::copy_constr(TMethodRunContext* run_context)
{
	TDynArr* obj = &run_context->object->get_as<TDynArr>();
	TDynArr* copy_from = &(*run_context->formal_params)[0].get_as<TDynArr>();

	SemanticApi::ISClass* el = copy_from->el_class;
	SemanticApi::ISMethod* el_copy_constr = el->GetCopyConstr();
	
	if (el_copy_constr != nullptr)
	{
		obj->Init();
		obj->el_class = el;
		if (copy_from->v->size() > 0)
		{
			obj->v->resize(copy_from->v->size());
			CallCopyConstr(*run_context->static_fields, &((*obj->v)[0]), &((*copy_from->v)[0]), 0, copy_from->v->size() / el->GetSize(), el->GetSize(), el, el_copy_constr);
		}
	}
	else
	{
		memset((TDynArr*)run_context->object->get(), 0xfeefee, sizeof(TDynArr));
		obj->Init(*copy_from);
	}
}

void TDynArr::assign_op(TMethodRunContext* run_context)
{
	TDynArr* left = ((TDynArr*)(*run_context->formal_params)[0].get());
	TDynArr* right = ((TDynArr*)(*run_context->formal_params)[1].get());

	std::vector<SemanticApi::ISMethod*> ops;

	left->el_class->GetOperators(ops, Lexer::TOperator::Assign);
	//TODO поиск нужного оператора присваивания
	dyn_arr_resize(*run_context->static_fields, left, right->v->size() / left->el_class->GetSize());
	if (left->v->size() > 0)
	{
		if(ops.size()>0)
			CallAssignOp(*run_context->static_fields, &((*left->v)[0]), &((*right->v)[0]), 0, left->v->size() / left->el_class->GetSize(), left->el_class->GetSize(), left->el_class, ops[0]);
		else
		{
			memcpy(&((*left->v)[0]), &((*right->v)[0]), left->v->size()*sizeof(int));
		}
	}
}

void TDynArr::get_element_op(TMethodRunContext* run_context)
{
	TDynArr* obj = ((TDynArr*)(*run_context->formal_params)[0].get());
	SemanticApi::ISClass* el = obj->el_class;
	run_context->result->SetAsReference(&((*obj->v)[el->GetSize()*(*(int*)(*run_context->formal_params)[1].get())]));
}

void TDynArr::EmplaceBack(std::vector<TStaticValue> &static_fields)
{
	dyn_arr_resize(static_fields, this, GetSize()+1);
}

void* TDynArr::GetElement(int i)
{
	return &v->at(el_class->GetSize()*i);
}
int TDynArr::GetSize()
{
	return v->size() / el_class->GetSize();
}

void TDynArr::resize(TMethodRunContext* run_context)
{
	TDynArr* obj = ((TDynArr*)run_context->object->get());
	int new_size = *(int*)(*run_context->formal_params)[0].get();
	
	dyn_arr_resize(*run_context->static_fields, obj, new_size);
}

void TDynArr::get_size(TMethodRunContext* run_context)
{
	TDynArr* obj = ((TDynArr*)(run_context->object->get()));
	*(int*)run_context->result->get() = obj->v->size() / obj->el_class->GetSize();
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
	
	decl.def_constr = TDynArr::constructor;	
	decl.copy_constr = TDynArr::copy_constr;
	decl.destr = TDynArr::destructor;

	decl.operators[(int)Lexer::TOperator::GetArrayElement] = TDynArr::get_element_op;
	decl.operators[(int)Lexer::TOperator::Assign] = TDynArr::assign_op;
	
	decl.methods.insert(std::make_pair(std::string("resize"), TDynArr::resize));
	decl.methods.insert(std::make_pair(std::string("size"), TDynArr::get_size));
	return decl;
}
