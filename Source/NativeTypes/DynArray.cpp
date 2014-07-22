	#include "DynArray.h"

#include "Semantic/FormalParam.h"
#include "Semantic/SClass.h"
#include "syntaxAnalyzer.h"
#include "Syntax/Statements.h"

#pragma push_macro("new")
#undef new

void TDynArr::constructor(std::vector<TStaticValue> &static_fields, std::vector<TStackValue> &formal_params, TStackValue& result, TStackValue& object)
{
	TDynArr* obj = new ((TDynArr*)object.get()) TDynArr();
}

void TDynArr::destructor(std::vector<TStaticValue> &static_fields, std::vector<TStackValue> &formal_params, TStackValue& result, TStackValue& object)
{
	TDynArr* obj = ((TDynArr*)object.get());
	obj->~TDynArr();
}

void TDynArr::copy_constr(std::vector<TStaticValue> &static_fields, std::vector<TStackValue> &formal_params, TStackValue& result, TStackValue& object)
{
	TDynArr* obj = new ((TDynArr*)object.get()) TDynArr(*(TDynArr*)formal_params[0].get());
}

void TDynArr::assign_op(std::vector<TStaticValue> &static_fields, std::vector<TStackValue> &formal_params, TStackValue& result, TStackValue& object)
{
	TDynArr* obj = ((TDynArr*)formal_params[0].get());
	*obj = *(TDynArr*)formal_params[1].get();
}

void TDynArr::get_element_op(std::vector<TStaticValue> &static_fields, std::vector<TStackValue> &formal_params, TStackValue& result, TStackValue& object)
{
	TDynArr* obj = ((TDynArr*)formal_params[0].get());
	result.SetAsReference(&obj->v[(*(int*)formal_params[1].get())]);
}

void TDynArr::resize(std::vector<TStaticValue> &static_fields, std::vector<TStackValue> &formal_params, TStackValue& result, TStackValue& object)
{
	TDynArr* obj = ((TDynArr*)object.get());
	obj->v.resize(*(int*)formal_params[0].get());
}

void TDynArr::get_size(std::vector<TStaticValue> &static_fields, std::vector<TStackValue> &formal_params, TStackValue& result, TStackValue& object)
{
	TDynArr* obj = ((TDynArr*)object.get());
	*(int*)result.get() = obj->v.size();
}

#pragma pop_macro("new")

void TDynArr::DeclareExternalClass(TSyntaxAnalyzer* syntax)
{
	TClass* cl = new TClass(syntax->base_class);
	syntax->lexer.ParseSource(
		"class extern TDynArray<T>\n"
		"{\n"
		"constr();\n"
		"constr(T& copy_from);\n"
		"destr();\n"
		"operator static [](TDynArray& v,int id):&T;\n"
		"operator static =(TDynArray& v,TDynArray& l);\n"
		"func resize(int new_size);\n"
		"func size:int;\n"
		"}\n"
		);
	cl->AnalyzeSyntax(syntax->lexer);

	TSClass* scl = new TSClass(syntax->sem_base_class, cl);
	syntax->sem_base_class->AddClass(scl);
	scl->Build();
	scl->SetSize(_INTSIZEOF(sizeof(TDynArr)));

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