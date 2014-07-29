	#include "DynArray.h"

#include "Semantic/FormalParam.h"
#include "Semantic/SClass.h"

#include "syntaxAnalyzer.h"

#include "Syntax/Statements.h"
#include "Syntax/Method.h"

#pragma push_macro("new")
#undef new

void TDynArr::constructor(std::vector<TStaticValue> &static_fields, std::vector<TStackValue> &formal_params, TStackValue& result, TStackValue& object)
{
	memset((TDynArr*)object.get(), 0xfeefee, sizeof(TDynArr));
	TDynArr* obj = new ((TDynArr*)object.get()) TDynArr();
	obj->el_class = object.GetClass()->GetTemplateParam(0).type;
}

void CallMethod(std::vector<TStaticValue> &static_fields, int* v, int first_element, int el_count, int el_size, TSClass* el_class, TSMethod* method)
{
	for (int i = first_element*el_size; i<el_count*el_size; i += el_size)
	{
		TStackValue el_obj(true, el_class);
		std::vector<TStackValue> without_params;
		TStackValue without_result;
		
		el_obj.SetAsReference(&v[i]);

		method->Run(static_fields, without_params, without_result, el_obj);
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
		
		method->Run(static_fields, params, without_result, el_obj);
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
		method->Run(static_fields, params, without_result, el_obj);
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

void TDynArr::destructor(std::vector<TStaticValue> &static_fields, std::vector<TStackValue> &formal_params, TStackValue& result, TStackValue& object)
{
	TDynArr* obj = ((TDynArr*)object.get());
	TSClass* el = obj->el_class;
	TSMethod* el_destr = el->GetDestructor();
	if (el_destr != NULL)
	{
		if (obj->v->size() > 0)
		{
			CallMethod(static_fields, &((*obj->v)[0]), 0, obj->v->size() / el->GetSize(), el->GetSize(), el, el_destr);
		}
	}
	obj->~TDynArr();
	memset((TDynArr*)object.get(), 0xfeefee, sizeof(TDynArr));
}

void TDynArr::copy_constr(std::vector<TStaticValue> &static_fields, std::vector<TStackValue> &formal_params, TStackValue& result, TStackValue& object)
{
	TDynArr* obj = ((TDynArr*)object.get());
	TDynArr* copy_from = (TDynArr*)formal_params[0].get();

	TSClass* el = copy_from->el_class;
	TSMethod* el_copy_constr = el->GetCopyConstr();
	
	if (el_copy_constr != NULL)
	{
		new (obj)TDynArr();
		obj->el_class = el;
		if (copy_from->v->size() > 0)
		{
			obj->v->resize(copy_from->v->size());
			CallCopyConstr(static_fields, &((*obj->v)[0]), &((*copy_from->v)[0]), 0, copy_from->v->size() / el->GetSize(), el->GetSize(), el, el_copy_constr);
		}
	}
	else
	{
		memset((TDynArr*)object.get(), 0xfeefee, sizeof(TDynArr));
		new (obj)TDynArr(*copy_from);
	}
}

void TDynArr::assign_op(std::vector<TStaticValue> &static_fields, std::vector<TStackValue> &formal_params, TStackValue& result, TStackValue& object)
{
	TDynArr* left = ((TDynArr*)formal_params[0].get());
	TDynArr* right = ((TDynArr*)formal_params[1].get());

	std::vector<TSMethod*> ops;

	left->el_class->GetOperators(ops, TOperator::Assign);
	//TODO поиск нужного оператора присваивания
	dyn_arr_resize(static_fields, left, right->v->size() / left->el_class->GetSize());
	if (left->v->size() > 0)
	{
		if(ops.size()>0&&!ops[0]->GetSyntax()->IsBytecode())
			CallAssignOp(static_fields, &((*left->v)[0]), &((*right->v)[0]), 0, left->v->size() / left->el_class->GetSize(), left->el_class->GetSize(), left->el_class, ops[0]);
		else
		{
			memcpy(&((*left->v)[0]), &((*right->v)[0]), left->v->size()*sizeof(int));
		}
	}
}

void TDynArr::get_element_op(std::vector<TStaticValue> &static_fields, std::vector<TStackValue> &formal_params, TStackValue& result, TStackValue& object)
{
	TDynArr* obj = ((TDynArr*)formal_params[0].get());
	TSClass* el = obj->el_class;
	result.SetAsReference(&((*obj->v)[el->GetSize()*(*(int*)formal_params[1].get())]));
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

void TDynArr::resize(std::vector<TStaticValue> &static_fields, std::vector<TStackValue> &formal_params, TStackValue& result, TStackValue& object)
{
	TDynArr* obj = ((TDynArr*)object.get());
	int new_size = *(int*)formal_params[0].get();
	
	dyn_arr_resize(static_fields, obj, new_size);
}

void TDynArr::get_size(std::vector<TStaticValue> &static_fields, std::vector<TStackValue> &formal_params, TStackValue& result, TStackValue& object)
{
	TDynArr* obj = ((TDynArr*)object.get());
	*(int*)result.get() = obj->v->size()/obj->el_class->GetSize();
}

#pragma pop_macro("new")

void TDynArr::DeclareExternalClass(TSyntaxAnalyzer* syntax)
{
	TClass* cl = new TClass(syntax->base_class);
	syntax->lexer.ParseSource(
		"class extern TDynArray<T>\n"
		"{\n"
		"constr();\n"
		"constr(TDynArray& copy_from);\n"
		"destr();\n"
		"operator static [](TDynArray& v,int id):&T;\n"
		"operator static =(TDynArray& v,TDynArray& l);\n"
		"func resize(int new_size);\n"
		"func size:int;\n"
		"}\n"
		);
	cl->AnalyzeSyntax(syntax->lexer);
	syntax->lexer.GetToken(TTokenType::Done);

	TSClass* scl = new TSClass(syntax->sem_base_class, cl);
	syntax->sem_base_class->AddClass(scl);
	scl->Build();

	scl->SetSize(IntSizeOf(sizeof(TDynArr))/sizeof(int));

	std::vector<TSMethod*> m;

	m.clear();
	scl->GetUserConstructors(m);
	m[0]->SetAsExternal(TDynArr::constructor);
	m[1]->SetAsExternal(TDynArr::copy_constr);
	scl->GetDestructor()->SetAsExternal(TDynArr::destructor);

	m.clear();
	scl->GetOperators(m, TOperator::GetArrayElement);
	m[0]->SetAsExternal(TDynArr::get_element_op);
	
	m.clear();
	scl->GetOperators(m, TOperator::Assign);
	m[0]->SetAsExternal(TDynArr::assign_op);

	m.clear();
	scl->GetMethods(m, syntax->lexer.GetIdFromName("resize"));
	m[0]->SetAsExternal(TDynArr::resize);

	m.clear();
	scl->GetMethods(m, syntax->lexer.GetIdFromName("size"));
	m[0]->SetAsExternal(TDynArr::get_size);
}





		//int* TDynArr::Get(int i)
		//{
		//	return &v[i*el_size];
		//}
		//int TDynArr::GetHigh()
		//{
		//	return (v.GetHigh()+1)/el_size-1;
		//}
		//void TDynArr::DefConstr(TVirtualMachine* machine,int use_methods_id)
		//{
		//	methods = &machine->GetArrayClassMethod(use_methods_id);
		//	el_size=methods->el_size;
		//	v.Init();
		//}
		//void TDynArr::CopyConstr(TVirtualMachine* machine, TDynArr* copy_from)
		//{
		//	methods=copy_from->methods;
		//	el_size=copy_from->el_size;
		//	v.Init();
		//	SetHigh(machine,copy_from->v.GetCount()/el_size-1,false);
		//	if(v.GetHigh()>-1)
		//	{
		//		int el_copy_constr=methods->el_copy_constr;
		//		if(el_copy_constr!=-1)
		//			machine->ArrayElementsRCopyConstr(&v[0],&copy_from->v[0],el_size,v.GetCount()/el_size,el_copy_constr);
		//		else memcpy(&v[0],&copy_from->v[0],v.GetCount()*sizeof(int));
		//	}
		//}
		//void TDynArr::Destr(TVirtualMachine* machine)
		//{
		//	int el_destr=methods->el_destr;
		//	if(el_destr!=-1&&v.GetHigh()>-1)
		//		machine->ArrayElementsDestr(&v[0],el_size,((v.GetHigh()+1)/el_size),el_destr);
		//	v.~TVector();
		//}
		//void TDynArr::AssignOp(TVirtualMachine* machine, TDynArr* right)
		//{
		//	if(v.GetHigh()!=right->v.GetHigh())
		//		SetHigh(machine,right->v.GetHigh());
		//	int el_rr_assign_op=methods->el_assign_op[true];
		//	int el_rv_assign_op=methods->el_assign_op[false];
		//	if(v.GetHigh()>-1)
		//	{
		//		if(el_rr_assign_op!=-1)
		//			machine->ArrayElementsRRAssignOp(&v[0],&right->v[0],el_size,(v.GetCount()/el_size),el_rr_assign_op);
		//		else if(el_rv_assign_op!=-1)
		//			machine->ArrayElementsRVAssignOp(&v[0],&right->v[0],el_size,(v.GetCount()/el_size),el_rv_assign_op,methods->el_copy_constr);
		//		else memcpy(&v[0],&right->v[0],v.GetCount()*sizeof(int));
		//	}
		//}
		//bool TDynArr::EqualOp(TVirtualMachine* machine, TDynArr* right)
		//{
		//	bool result=true;
		//	int el_equal_op=methods->el_equal_op[true][true];//TODO выбор оптимального оператора
		//	if(v.GetHigh()!=right->v.GetHigh())result=false;
		//	else if(el_equal_op!=-1)
		//		result=machine->ArrayElementsRREqualOp(&v[0],&right->v[0],el_size,(v.GetCount()/el_size),el_equal_op);
		//	else if(v.GetHigh()>-1) result=(memcmp(&v[0],&right->v[0],v.GetCount()*sizeof(int))==0);
		//	else result=true;
		//	return result;
		//}
		//void TDynArr::SetHigh(TVirtualMachine* machine,int new_high,bool call_constr_destr)
		//{
		//	int high=((v.GetHigh()+1)/el_size)-1;
		//	if(new_high==high)return;
		//	if(call_constr_destr)
		//	{
		//		int el_destr=methods->el_destr;
		//		if(el_destr!=-1&&new_high<high)
		//			machine->ArrayElementsDestr(&v[0],el_size,high-new_high,el_destr,new_high+1);
		//	}
		//	v.SetCount((new_high+1)*el_size);
		//	if(call_constr_destr)
		//	{
		//		int el_def_constr=methods->el_def_constr;
		//		if(el_def_constr!=-1&&new_high>high)
		//			machine->ArrayElementsDefConstr(&v[0],el_size,new_high-high,el_def_constr,high+1);
		//	}
		//}
		//int* TDynArr::IncHigh(TVirtualMachine* machine,int count)
		//{
		//	SetHigh(machine,(v.GetHigh()+1)/el_size-1+count);
		//	return &v[v.GetHigh()-el_size+1];
		//}