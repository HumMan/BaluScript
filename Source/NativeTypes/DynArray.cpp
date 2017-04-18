#include "DynArray.h"

#include "../Semantic/FormalParam.h"
#include "../Semantic/SClass.h"

#include "../syntaxAnalyzer.h"

#include "../Syntax/Statements.h"
#include "../Syntax/Method.h"
#include "../Syntax/Class.h"

#include <string.h>

void TDynArr::constructor(TMethodRunContext run_context)
{
	run_context.object->get_as<TDynArr>().Init();
	run_context.object->get_as<TDynArr>().el_class = run_context.object->GetClass()->GetTemplateParam(0).type;
}

void CallMethod(std::vector<TStaticValue> &static_fields, int* v, int first_element, int el_count, int el_size, TSClass* el_class, TSMethod* method)
{
	for (int i = first_element*el_size; i<el_count*el_size; i += el_size)
	{
		TStackValue el_obj(true, el_class);
		std::vector<TStackValue> without_params;
		TStackValue without_result;
		
		el_obj.SetAsReference(&v[i]);

		method->Run(TMethodRunContext(&static_fields, &without_params, &without_result, &el_obj));
	}
}

void CallCopyConstr(std::vector<TStaticValue> &static_fields, int* v, int* copy_from, int first_element, int el_count, int el_size, TSClass* el_class, TSMethod* method)
{
	for (int i = first_element*el_size; i<el_count*el_size; i += el_size)
	{
		TStackValue el_obj(true, el_class);
		el_obj.SetAsReference(&v[i]);

		std::vector<TStackValue> params;
		TStackValue without_result;
		params.resize(1);
		params[0] = TStackValue(true, el_class);
		params[0].SetAsReference(&copy_from[i]);
		
		method->Run(TMethodRunContext(&static_fields, &params, &without_result, &el_obj));
	}
}

void CallAssignOp(std::vector<TStaticValue> &static_fields, int* left, int* right, int first_element, int el_count, int el_size, TSClass* el_class, TSMethod* method)
{
	for (int i = first_element*el_size; i<el_count*el_size; i += el_size)
	{
		TStackValue el_obj;
		std::vector<TStackValue> params;
		TStackValue without_result;
		params.resize(2);
		params[0] = TStackValue(true, el_class);
		params[0].SetAsReference(&left[i]);
		params[1] = TStackValue(true, el_class);
		params[1].SetAsReference(&right[i]);
		method->Run(TMethodRunContext(&static_fields, &params, &without_result, &el_obj));
	}
}

void dyn_arr_resize(std::vector<TStaticValue> &static_fields, TDynArr* obj, int new_size)
{
	TSClass* el = obj->el_class;
	int curr_size = obj->v->size() / el->GetSize();

	if (curr_size == new_size)
		return;

	if (curr_size > new_size)
	{
		TSMethod* el_destr = el->GetDestructor();
		if (el_destr != NULL)
		{
			CallMethod(static_fields, &((*obj->v)[0]), new_size, curr_size, el->GetSize(), el, el_destr);
		}
	}
	obj->v->resize(el->GetSize()*new_size);
	if (curr_size < new_size)
	{
		TSMethod* el_def_constr = el->GetDefConstr();
		if (el_def_constr != NULL)
		{
			CallMethod(static_fields, &((*obj->v)[0]), curr_size, new_size, el->GetSize(), el, el_def_constr);
		}
	}
}

void TDynArr::destructor(TMethodRunContext run_context)
{
	TDynArr* obj = ((TDynArr*)run_context.object->get());
	TSClass* el = obj->el_class;
	TSMethod* el_destr = el->GetDestructor();
	if (el_destr != NULL)
	{
		if (obj->v->size() > 0)
		{
			CallMethod(*run_context.static_fields, &((*obj->v)[0]), 0, obj->v->size() / el->GetSize(), el->GetSize(), el, el_destr);
		}
	}
	obj->~TDynArr();
	memset((TDynArr*)run_context.object->get(), 0xfeefee, sizeof(TDynArr));
}

void TDynArr::copy_constr(TMethodRunContext run_context)
{
	TDynArr* obj = &run_context.object->get_as<TDynArr>();
	TDynArr* copy_from = &(*run_context.formal_params)[0].get_as<TDynArr>();

	TSClass* el = copy_from->el_class;
	TSMethod* el_copy_constr = el->GetCopyConstr();
	
	if (el_copy_constr != NULL)
	{
		obj->Init();
		obj->el_class = el;
		if (copy_from->v->size() > 0)
		{
			obj->v->resize(copy_from->v->size());
			CallCopyConstr(*run_context.static_fields, &((*obj->v)[0]), &((*copy_from->v)[0]), 0, copy_from->v->size() / el->GetSize(), el->GetSize(), el, el_copy_constr);
		}
	}
	else
	{
		memset((TDynArr*)run_context.object->get(), 0xfeefee, sizeof(TDynArr));
		obj->Init(*copy_from);
	}
}

void TDynArr::assign_op(TMethodRunContext run_context)
{
	TDynArr* left = ((TDynArr*)(*run_context.formal_params)[0].get());
	TDynArr* right = ((TDynArr*)(*run_context.formal_params)[1].get());

	std::vector<TSMethod*> ops;

	left->el_class->GetOperators(ops, Lexer::TOperator::Assign);
	//TODO поиск нужного оператора присваивания
	dyn_arr_resize(*run_context.static_fields, left, right->v->size() / left->el_class->GetSize());
	if (left->v->size() > 0)
	{
		if(ops.size()>0)
			CallAssignOp(*run_context.static_fields, &((*left->v)[0]), &((*right->v)[0]), 0, left->v->size() / left->el_class->GetSize(), left->el_class->GetSize(), left->el_class, ops[0]);
		else
		{
			memcpy(&((*left->v)[0]), &((*right->v)[0]), left->v->size()*sizeof(int));
		}
	}
}

void TDynArr::get_element_op(TMethodRunContext run_context)
{
	TDynArr* obj = ((TDynArr*)(*run_context.formal_params)[0].get());
	TSClass* el = obj->el_class;
	run_context.result->SetAsReference(&((*obj->v)[el->GetSize()*(*(int*)(*run_context.formal_params)[1].get())]));
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

void TDynArr::resize(TMethodRunContext run_context)
{
	TDynArr* obj = ((TDynArr*)run_context.object->get());
	int new_size = *(int*)(*run_context.formal_params)[0].get();
	
	dyn_arr_resize(*run_context.static_fields, obj, new_size);
}

void TDynArr::get_size(TMethodRunContext run_context)
{
	TDynArr* obj = ((TDynArr*)(run_context.object->get()));
	*(int*)run_context.result->get() = obj->v->size() / obj->el_class->GetSize();
}

void TDynArr::DeclareExternalClass(TSyntaxAnalyzer* syntax)
{
	SyntaxInternal::TClass* cl = new SyntaxInternal::TClass(syntax->GetBaseClass());
	syntax->GetBaseClass()->AddNested(cl);
	syntax->GetLexer()->ParseSource(
		"class extern TDynArray<T>\n"
		"{\n"
		"default();\n"
		"copy(TDynArray& copy_from);\n"
		"destr();\n"
		"operator static [](TDynArray& v,int id):&T;\n"
		"operator static =(TDynArray& v,TDynArray& l);\n"
		"func resize(int new_size);\n"
		"func size:int;\n"
		"}\n"
		);
	cl->AnalyzeSyntax(syntax->GetLexer());
	syntax->GetLexer()->GetToken(Lexer::TTokenType::Done);

	TSClass* scl = new TSClass(syntax->GetCompiledBaseClass(), cl);
	syntax->GetCompiledBaseClass()->AddClass(scl);
	scl->Build();

	scl->SetSize(LexerIntSizeOf(sizeof(TDynArr)) / sizeof(int));
	scl->SetAutoMethodsInitialized();

	scl->GetDefConstr()->SetAsExternal(TDynArr::constructor);
	
	std::vector<TSMethod*> m;
	m.clear();
	scl->GetCopyConstructors(m);
	m[0]->SetAsExternal(TDynArr::copy_constr);
	scl->GetDestructor()->SetAsExternal(TDynArr::destructor);

	
	m.clear();
	scl->GetOperators(m, Lexer::TOperator::GetArrayElement);
	m[0]->SetAsExternal(TDynArr::get_element_op);
	
	m.clear();
	scl->GetOperators(m, Lexer::TOperator::Assign);
	m[0]->SetAsExternal(TDynArr::assign_op);

	m.clear();
	scl->GetMethods(m, syntax->GetLexer()->GetIdFromName("resize"));
	m[0]->SetAsExternal(TDynArr::resize);

	m.clear();
	scl->GetMethods(m, syntax->GetLexer()->GetIdFromName("size"));
	m[0]->SetAsExternal(TDynArr::get_size);
}
